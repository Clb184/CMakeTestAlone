rm -rf build
mkdir build
cd build

cmake ../ && make && ./CMAKE_TEST
