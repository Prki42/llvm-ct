{
  description = "LLVM constant-time";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forAllSystems =
        f:
        nixpkgs.lib.genAttrs supportedSystems (
          system:
          f {
            pkgs = import nixpkgs { inherit system; };
          }
        );
    in
    {
      devShells = forAllSystems (
        { pkgs }:
        let
          llvmPkgs = pkgs.llvmPackages_22;
          lit = pkgs.python3Packages.buildPythonApplication {
            pname = "lit";
            version = "22.1.7";
            pyproject = true;
            build-system = [ pkgs.python3Packages.setuptools ];
            src = pkgs.fetchgit {
              url = "https://github.com/llvm/llvm-project.git";
              rev = "llvmorg-22.1.7";
              sparseCheckout = [ "llvm/utils/lit" ];
              /*
                only used on the first run to get the real hash to put below
              */
              # hash = pkgs.lib.fakeHash;
              hash = "sha256-rGfANVfUWXcbhSsb3byfFKiyZ385SF1RpXSqlA0zgPA=";
            };
            sourceRoot = "llvm-project/llvm/utils/lit";
          };
        in
        {
          default = pkgs.mkShell.override { stdenv = llvmPkgs.stdenv; } {
            packages = [
              llvmPkgs.llvm
              pkgs.cmake
              lit
            ];

            /*
              nix clang wrapper injects -isystem for llvm includes at
              invocation time so compilation works, but compile_commands.json
              records commands before the wrapper acts so clangd can't find
              them. this env var gets picked up by cmake and written into
              compile_commands.json so clangd (LSP) sees the paths too
            */
            env.CXXFLAGS = "-isystem ${llvmPkgs.llvm.dev}/include";
          };
        }
      );
    };
}
