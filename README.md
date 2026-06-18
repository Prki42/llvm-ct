# KK

## Dependencies

**Fedora:**
```sh
dnf install llvm llvm-devel clang cmake
```

**Arch:**
```sh
pacman -S llvm clang cmake
```

**Ubuntu/Debian:**
```sh
apt install llvm-dev clang cmake
```

## Building

```sh
cmake -S . -B build
cmake --build build
```

## Running

```sh
clang -S -emit-llvm <input.c> -o <input.ll>
opt --load-pass-plugin=build/CTPass.so --passes="ct-branch" -disable-output <input.ll>
```

## Development

### `clangd` setup

Add `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` to `cmake` configuration command to generate `build/compile_commands.json`:

```sh
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Project structure

- `include/ArgDepAnalysis.h` - analysis pass
- `include/CTBranchPass.h` - transformation pass
- `src/Plugin.cpp` - pass registration and plugin entry point

### Testing

Project utilizes LLVM's `lit` and `FileCheck` tools for testing.

`FileCheck` is usually included in some `llvm` package but `lit` should be installed from [LLVM repo](https://github.com/llvm/llvm-project/tree/llvmorg-22.1.7/llvm/utils/lit).

To run the test use:
```sh
# run all tests
lit test/ -v

# run some tests
lit test/analysis -v
lit test/.../test_file.ll -v
```

### Useful resources

- LLVM User Guides:
  - [Writing an LLVM Pass](https://llvm.org/docs/WritingAnLLVMNewPMPass.html)
  - [Building LLVM with CMake](https://llvm.org/docs/CMake.html)
  - [lit - LLVM Integrated Tester](https://llvm.org/docs/CommandGuide/lit.html)
  - [FileCheck - Flexible pattern matching file verifier](https://llvm.org/docs/CommandGuide/FileCheck.html)
- [LLVM repo](https://github.com/llvm/llvm-project)
