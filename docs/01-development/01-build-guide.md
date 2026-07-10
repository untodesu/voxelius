# Development: build guide

## The easy way

### Debug builds

|Target|Configuration|Command|  
|----|----|----|  
|Windows on 64-bit x86|Debug|`scripts/build-windows-x64-debug.bat`|  
|Windows on 32-bit x86|Debug|`scripts/build-windows-x32-debug.bat`|  
|Anything with a POSIX shell|Debug|`scripts/build-generic-debug.sh`|  

### Release builds

|Target|Configuration|Command|  
|----|----|----|  
|Windows on 64-bit x86|Release|`scripts/build-windows-x64-release.bat`|  
|Windows on 32-bit x86|Release|`scripts/build-windows-x32-release.bat`|  
|Anything with a POSIX shell|Release|`scripts/build-generic-release.sh`|  

## The hard way

### Debug builds

```
cmake -B build -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --parallel
```

### Release builds

```
cmake -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```
