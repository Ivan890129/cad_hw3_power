# cad_hw3_power

all:311510207.cpp

g++ -std=c++11 311510207.cpp -o 311510207.o
clean:

rm -f 311510207.o
./311510207.cpp "example.v" -l "test_lib.lib"
