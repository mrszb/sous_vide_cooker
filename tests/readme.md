windows

```
cmake -B build .
```

```
cmake --build ./build  --config Release
```

```
cd build
ctest -V
```



linux:

```
cmake -B build -G "Unix Makefiles"
cd build
make
ctest -V
```

