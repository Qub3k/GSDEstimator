# GSD Estimator

Estimator for [Generalized Score Distibution's](https://arxiv.org/abs/1909.04369) parameters using OpenCL for accelerating calculation on GPI.

## Build process
In case of doubts refere to [wiki page](https://github.com/srokadev/GSDEstimator/wiki).

### Requirements
For windows you'll need Visual Studio Build Tools.
For linux just these `g++` `make` `cmake` should be enough.

### Command sequence

```
mkdir build
cmake -B build
cmake --build build --config Release
```

For linux the final executable named GSDEsimator will end up in build directory.
For windows you will find it in build/Release directory.

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
