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
          };
        }
      );
    };
}
