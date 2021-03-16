
all: MLE
	g++ -Wall -Werror -O3 main.cpp MLE.o -o GSDEstimator

MLE: MLE.cpp
	g++ -Wall -Werror -O3 -c MLE.cpp
