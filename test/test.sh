rm -rf ./build
mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc\
 -DCMAKE_CXX_COMPILER=arm-linux-gnueabi-g++
make
qemu-arm -L $LINARO_SYSROOT ./JITtest