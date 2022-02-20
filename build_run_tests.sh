cmake -B build .
cmake --build ./build  --target clean
cmake --build ./build  --config Release
cd build
ctest -V

