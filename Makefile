DBG=1

linux: main.cpp pal.cpp
	g++ $^ -O3 -DDEBUG=${DBG} -g -lOpenCL -I. -o GSDEstimator

windows: main.cpp pal.cpp
	x86_64-w64-mingw32-g++ $^ -O3 -static -DDEBUG=${DBG} -Llib/x86_64 -lOpenCL -Iinclude -I. -o GSDEstimator
