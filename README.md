# JIT compiler
Completed by Andrei Bazhenov for CAOS and formal expressions cources.

Build and run:
```bash
mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc -DCMAKE_CXX_COMPILER=arm-linux-gnueabi-g++
qemu-arm -L $LINARO_SYSROOT ./JIT
```
