# GSD Estimator

Estimator for [Generalized Score Distibution's](https://arxiv.org/abs/1909.04369) parameters using OpenCL for accelerating calculation on GPI.

## Requirements

### Linux

You need to install opencl library. The procedure will be different for every distro.\
For ubuntu just installing `ocl-icd-opencl-dev` package should work.

### Windows
For windows everything works out of the box.

## Usage

```
Usage: ./GSDEstimator <samples_file> [output_file]
	samples_file: name of input file containg samples for estimation
	output_file: name of csv file containing results
```

- samples file is a text file with each sample in new line. Every sample consits of 5 values from subjective experiments utilizing the Absolute Category Rating (ACR) technique, with possible answers {1,2,⋯,5} 
- output file will be a csv file with results decribed by values
  - `idx` - index of sample
  - `psi` - estimated mean (rating)
  - `rho` - estimated variance
  - `log_likelihood` - logarithm of likelihood value
