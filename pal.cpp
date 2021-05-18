#include <iostream>
#include "pal.hpp"

const char* source_code = " \
typedef struct __attribute__ ((packed)) { \
  float psi; \
  float rho; \
  float grades[8]; \
} grid_node; \
 \
__kernel void \
estimate( \
         __global grid_node *node, \
         __global int *samples_ptr, \
         __const int n_samples, \
         __global float *output) \
{ \
  size_t id = get_global_id(0); \
  float8 probs = log(vload8(0, node[id].grades)); \
  for (int i = 0; i < n_samples; ++i) { \
    int8 samples = vload8(i, samples_ptr); \
    float8 res = probs * convert_float8(samples); \
    float sum = res.s0 + res.s1 + res.s2 + res.s3 + res.s4; \
    output[n_samples*id + i] = sum; \
  } \
} \
";

#if DEBUG
const char* clErrorString(cl_int err);
#endif

template<typename... Args>
inline void clCheckError(cl_int err, const char* msg, Args... args) {
  if ( err != CL_SUCCESS ) {
#if DEBUG
    const char* error_string = clErrorString(err);
    fprintf(stderr, "Error: %d ( %s )\n", err, error_string);
    fprintf(stderr, msg, args...);
    fprintf(stderr, "\n");
#endif
    std::exit(err);
  }
}

Context::Context() {
  this->_input = this->_samples = this->_output = NULL;
}

int Context::init() {
  cl_platform_id platform_id;
  cl_uint num_of_platforms=0;
  cl_uint num_of_devices=0;

  // retreives a list of platforms available
  if (clGetPlatformIDs(1, &platform_id, &num_of_platforms)!= CL_SUCCESS)
  {
      std::cerr << "Unable to get platform_id" << std::endl;
      return 1;
  }

  // try to get a supported GPU device
  if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &this->_device_id, &num_of_devices) != CL_SUCCESS)
  {
      std::cerr << "Unable to get device_id" << std::endl;
      return 1;
  }

  clGetDeviceInfo(this->_device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(this->_max_allocation), (void*)&this->_max_allocation, NULL);
  this->_max_allocation *= 0.7;
#if DEBUG
  std::cerr << "CL_DEVICE_MAX_MEM_ALLOC_SIZE: " << this->_max_allocation/1024/1024 << std::endl;
#endif

  // context properties list - must be terminated with
  cl_context_properties properties[3];
  properties[0]= CL_CONTEXT_PLATFORM;
  properties[1]= (cl_context_properties) platform_id;
  properties[2]= 0;

  // create a context with the GPU device
  cl_int err;
  this->_cl_context = clCreateContext(properties, 1, &this->_device_id, NULL, NULL, &err);

  // create command queue using the context and device
  this->_command_queue = clCreateCommandQueue(this->_cl_context, this->_device_id, 0, &err);

  return 0;
}

//int Context::prepare_kernel(const char* function_name, const char* source) {
int Context::prepare_kernel() {
  cl_int err;
  // create a program from the kernel source code
  cl_program program = clCreateProgramWithSource(this->_cl_context, 1, &source_code, NULL, &err);
  clCheckError(err, "clCreateProgramWithSource failed");

  // compile the program
  int ret;
  if ( (ret = clBuildProgram(program, 0, NULL, NULL, NULL, NULL)) != CL_SUCCESS )
  {
    // Determine the size of the log
    size_t log_size;
    clGetProgramBuildInfo(program, this->_device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *log = (char *) malloc(log_size);
    clGetProgramBuildInfo(program, this->_device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

    // Print the log
    std::cerr << log << std::endl;
    free(log);
    return 1;
  }

  // specify which kernel from the program to execute
  this->_kernel = clCreateKernel(program, "estimate", &err);
  clCheckError(err, "clCreateKernel failed");

  clReleaseProgram(program);
  return 0;
}

size_t Context::setup_buffers(Sample* samples_ptr, size_t n_samples, GridNode* input_data, size_t size) {
  cl_int err;
  this->_work_size = size;
  this->_n_samples = std::min(this->_max_allocation / (sizeof(float) * this->_work_size), n_samples);

  // create buffers for the input and ouput
  if (this->_input == NULL) {
    this->_input = clCreateBuffer(this->_cl_context, CL_MEM_READ_ONLY, sizeof(GridNode) * this->_work_size, NULL, &err);
    clCheckError(err, "clCreateBuffer on input failed. Tried allocating: %lu", sizeof(GridNode) * this->_work_size);
    // load data into the input buffer
    err = clEnqueueWriteBuffer(this->_command_queue, this->_input, CL_TRUE, 0, sizeof(GridNode) * this->_work_size, input_data, 0, NULL, NULL);
    clCheckError(err, "clEnqueueWriteBuffer on input failed");
  }

  if (this->_samples == NULL) {
    this->_samples = clCreateBuffer(this->_cl_context, CL_MEM_READ_ONLY, sizeof(Sample) * this->_n_samples, NULL, &err);
    clCheckError(err, "clCreateBuffer on samples failed. Tried allocating: %ld", sizeof(Sample) * this->_n_samples);
  }

  if (this->_output == NULL) {
    this->_output = clCreateBuffer(this->_cl_context, CL_MEM_WRITE_ONLY, sizeof(float) * this->_work_size * this->_n_samples, NULL, &err);
    clCheckError(err, "clCreateBuffer on output failed. Tried allocating: %lu", sizeof(float) * this->_work_size * this->_n_samples / 1024 / 1024);
  }

  err = clEnqueueWriteBuffer(this->_command_queue, this->_samples, CL_TRUE, 0, sizeof(Sample) * this->_n_samples, samples_ptr, 0, NULL, NULL);
  clCheckError(err, "clEnqueueWriteBuffer on samples failed");

  // set the argument list for the kernel command
  cl_int samples = this->_n_samples;
  clSetKernelArg(this->_kernel, 0, sizeof(cl_mem), &this->_input);
  clSetKernelArg(this->_kernel, 1, sizeof(cl_mem), &this->_samples);
  clSetKernelArg(this->_kernel, 2, sizeof(cl_int), &samples);
  clSetKernelArg(this->_kernel, 3, sizeof(cl_mem), &this->_output);

  return this->_n_samples;
}

void Context::start() {
  // enqueue the kernel command for execution
  cl_int err = clEnqueueNDRangeKernel(this->_command_queue, this->_kernel, 1, NULL, &this->_work_size, NULL, 0, NULL, NULL);
  clCheckError(err, "clEnqueueNDRangeKernel failed");
}

void Context::read_data(float* ptr) {
  cl_int err = clEnqueueReadBuffer(this->_command_queue, this->_output, CL_TRUE, 0, sizeof(float) * this->_work_size * this->_n_samples, ptr, 0, NULL, NULL);
  clCheckError(err, "clEnqueueReadBuffer failed");
}

void Context::join() {
  cl_int err = clFinish(this->_command_queue);
  clCheckError(err, "clFinish failed");
}

Context::~Context() {
  clReleaseKernel(this->_kernel);
  clReleaseCommandQueue(this->_command_queue);
  clReleaseContext(this->_cl_context);
  clReleaseMemObject(this->_input);
  clReleaseMemObject(this->_samples);
  clReleaseMemObject(this->_output);
}

#if DEBUG
const char* clErrorString(cl_int err) {
  switch(err){
    // run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
  }
}
#endif
