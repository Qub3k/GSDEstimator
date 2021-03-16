
all: MLE
	g++ main.cpp MLE.o -o GSDEstimator

MLE: MLE.cpp
	g++ -c MLE.cpp
