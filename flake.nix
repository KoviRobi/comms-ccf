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
        devShell = { demo ? false, recording ? false }: pkgs.mkShell {
          packages = [
            pkgs.stdenv.cc
            pkgs.clang-tools
            pkgs.cmake
            pkgs.ninja
            pkgs.black
            pkgs.pyright
            pkgs.isort
            pkgs.uv
            pkgs.python3
            pkgs.python3.pkgs.pyelftools
            pkgs.python3.pkgs.pyserial-asyncio
            pkgs.python3.pkgs.pytest
            pkgs.python3.pkgs.pyyaml
            pkgs.python3.pkgs.hypothesis
            pkgs.python3.pkgs.cobs
            pkgs.python3.pkgs.cbor2
            pkgs.python3.pkgs.fnv-hash-fast
            pkgs.python3.pkgs.squarify
            pkgs.python3.pkgs.intervaltree
            pkgs.python3.pkgs.cxxfilt
            pkgs.python3.pkgs.junit2html
          ]
          ++ pkgs.lib.optionals demo [
            # FreeRTOSDemo
            pkgs.gcc-arm-embedded
            pkgs.qemu
          ]
          ++ pkgs.lib.optionals recording [
            pkgs.asciinema
            pkgs.python3.pkgs.pynput
          ];
        };
      in
      {
        devShells = {
          light = devShell   { demo = false; recording = false; };
          default = devShell { demo = true;  recording = false; };
          full = devShell    { demo = true;  recording = true;  };
        };
      }
    );
}
