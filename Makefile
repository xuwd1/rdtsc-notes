

all:
	clang++ -O3 -Wno-error=register rdtscp.cpp -o rdtscp
	clang++ -O3 calibration.cpp -o calibration