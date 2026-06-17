# KK

## Dependencies

**Fedora:**
```
sudo dnf install llvm llvm-devel clang cmake
```

**Arch:**
```
sudo pacman -S llvm clang cmake
```

**Ubuntu/Debian:**
```
sudo apt install llvm-dev clang cmake
```

## Building

```
cmake -S . -B build
cmake --build build
```

## Running

```
clang -S -emit-llvm <input.c> -o <input.ll>
opt --load-pass-plugin=build/CTPass.so --passes="ct-branch" -disable-output <input.ll>
```

## Development

### `clangd` setup

Add `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` to `cmake` configuration command to generate `build/compile_commands.json`:

```
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Project structure

- `include/ArgDepAnalysis.h` - analysis pass
- `include/CTBranchPass.h` - transformation pass
- `src/Plugin.cpp` - pass registration and plugin entry point

### Useful resources

- LLVM User Guides:
  - [Writing an LLVM Pass](https://llvm.org/docs/WritingAnLLVMNewPMPass.html)
  - [Building LLVM with CMake](https://llvm.org/docs/CMake.html)
