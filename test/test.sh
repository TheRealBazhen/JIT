rm -rf ./build
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=arm-linux-gnueabi-g++
make
qemu-arm -L $LINARO_SYSROOT ./JITtest
cd CMakeFiles/JITtest.dir/
gcov *.o
lcov --directory . -c -o main.info
cd ../../
mkdir output
cd output
genhtml ../CMakeFiles/JITtest.dir/main.info
