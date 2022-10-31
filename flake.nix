{
  description = "Show recent used git branches";

  inputs.nixpkgs.url = "nixpkgs/nixos-22.05";

  outputs = { self, nixpkgs }:
    let
      pkgs = import nixpkgs { system = "x86_64-linux"; };

      git-recent = pkgs.stdenv.mkDerivation {
        pname = "git-recent";
        version = "1";
        src = ./.;

        buildInputs = [
          pkgs.cmake
          pkgs.ninja
          pkgs.libgit2
          pkgs.pkg-config
        ];
      };
    in
    {
      packages.x86_64-linux.git-recent = git-recent;
      packages.x86_64-linux.default = git-recent;
    };
}
