# KK

## Setup

The project provides a [Nix flake](https://nix.dev/concepts/flakes) with a dev shell that includes all dependencies:

```sh
nix develop
```

Alternatively, install LLVM 22 (including development headers), Clang, CMake, and [lit](https://github.com/llvm/llvm-project/tree/llvmorg-22.1.7/llvm/utils/lit) using your package manager.

## Building

```sh
cmake -S . -B build
cmake --build build
```

## Running

```sh
clang -S -emit-llvm <input.c> -o <input.ll>
opt --load-pass-plugin=build/CTPass.so --passes="mergereturn,structurizecfg,ct-branch,ct-data,simplifycfg" -S <in.ll> -o <out.ll>
```

## Development

### `clangd` setup

Add `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` to `cmake` configuration command to generate `build/compile_commands.json`:

```sh
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Debug builds

We use `LLVM_DEBUG()` for debug logs but since packaged `opt` is compiled in Release mode, we redefine `LLVM_DEBUG()` when CTPass.so is compiled in Debug mode. To build Debug version:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

Then to enable debug logs pass `--ct-verbose` to `opt` (simulates `opt`'s `--debug-only` flag)

### Generating examples from c files

```sh
clang -S -emit-llvm -fno-discard-value-names -O0 <input.c> -o <out.ll>
opt -passes=sroa,mem2reg -S <in.ll> -o <out.ll>
```

To visualize CFG of a function in a given `.ll` file:
```sh
# opens in default image viewer
./scripts/cfg-image.sh <in.ll>

# saves as png/svg file
./scripts/cfg-image.sh -f <png/svg> -o <image.png/svg> <in.ll>
```

### Running passes in debug mode

```sh
opt --load-pass-plugin=build/CTPass.so --ct-verbose --passes="mergereturn,structurizecfg,ct-branch,ct-data,simplifycfg" --verify-each -S <in.ll>
```

`simplifycfg` at the end is not required but since `ct-branch` transformation can produce a lot of unconditional jumps it makes output nicer.

### Project structure

- `src/ArgDepAnalysis.cpp` - argument dependance analysis pass
- `src/CTBranchPass.cpp` - if/else linearization pass
- `src/Plugin.cpp` - pass registration and plugin entry point

### Testing

Project utilizes LLVM's `lit` and `FileCheck` tools for some tests. To run those tests use:
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
  - [Regions](https://github.com/llvm/llvm-project/blob/8cdf6346f46c505928a9fb9d3ef9e8ce125a2108/llvm/include/llvm/Analysis/RegionInfo.h#L189)
  - [Structurize CFG](https://github.com/llvm/llvm-project/blob/8cdf6346f46c505928a9fb9d3ef9e8ce125a2108/llvm/lib/Transforms/Scalar/StructurizeCFG.cpp#L230)
  - [STL extras](https://github.com/llvm/llvm-project/blob/3a1eaf167e1dd9fb518a1a2d4a3838d159ea8f9f/llvm/include/llvm/ADT/STLExtras.h)
