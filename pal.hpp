#pragma once

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_TARGET_OPENCL_VERSION 300
extern "C" {
  #include <CL/cl.h>
}

struct GridNode {
  double psi;
  double rho;
  double grades[5];
};

struct Sample {
  int32_t multiplicity[5];
};

class Context {
  private:
    cl_device_id _device_id;
    cl_context _cl_context;
    cl_kernel _kernel;
    cl_command_queue _command_queue;
    cl_mem _input, _samples, _output;
    size_t _work_size;
    size_t _n_samples;
    size_t _max_allocation;

  public:
    Context();
    int init();
    int prepare_kernel();
    size_t setup_buffers(Sample* samples_ptr, size_t n_samples, GridNode* input_data, size_t size);
    void read_data(float* ptr);
    void start();
    void join();
    ~Context();
};
