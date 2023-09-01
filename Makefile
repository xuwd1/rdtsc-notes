

all:
	clang++ -O3 -Wno-error=register rdtscp.cpp -o rdtscp
	clang++ -O3 ryzen7840h.cpp -o ryzen7840h