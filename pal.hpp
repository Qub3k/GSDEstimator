#pragma once

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_TARGET_OPENCL_VERSION 300
extern "C" {
  #include <CL/cl.h>
}

constexpr size_t GRIDNODE_SIZE = sizeof(float) * 7;
struct GridNode {
  float psi;
  float rho;
  float grades[5];
private:
  float _filler[3];
};

struct Sample {
  int32_t multiplicity[5];
private:
  int32_t _filler[3];
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
