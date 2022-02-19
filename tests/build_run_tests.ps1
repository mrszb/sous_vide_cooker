Remove-Item -Recurse -Force build
cmake -B build .
cmake --build ./build  --target clean
cmake --build ./build  --config Release

