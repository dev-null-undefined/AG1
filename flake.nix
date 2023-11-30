{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
      };
      libraries = with pkgs; [
        gbenchmark
      ];
    in {
      cmake-helper = rec {
        libs = builtins.map builtins.toString (builtins.map pkgs.lib.getLib libraries);
        includes = builtins.map builtins.toString (builtins.map pkgs.lib.getDev libraries);
        cmake-file = pkgs.writeText "CMakeList.txt" (pkgs.lib.strings.concatLines (
          (builtins.map (lib: ''target_link_directories(''${CMAKE_PROJECT_NAME} PUBLIC ${lib}/lib)'') libs)
          ++ (builtins.map (include: ''include_directories(${include}/include)'') includes)
        ));
      };
    });
}
