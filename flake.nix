{
  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        demo = true;
      in
      {
        devShells.default = pkgs.mkShell {
          packages = [
            pkgs.stdenv.cc
            pkgs.clang-tools
            pkgs.cmake
            pkgs.ninja
            pkgs.black
            pkgs.pyright
            pkgs.isort
            pkgs.python3
            pkgs.python3.pkgs.pytest
            pkgs.python3.pkgs.hypothesis
            pkgs.python3.pkgs.cobs
            pkgs.python3.pkgs.cbor
            pkgs.python3.pkgs.fnv-hash-fast
          ]
          ++ pkgs.lib.optionals demo [
            # FreeRTOSDemo
            pkgs.gcc-arm-embedded
            pkgs.qemu
          ];
        };
      }
    );
}
