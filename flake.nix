
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

        # Meson is no longer able to pick up Boost automatically.
        # https://github.com/NixOS/nixpkgs/issues/86131
        BOOST_INCLUDEDIR = "${pkgs.lib.getDev pkgs.boost}/include";
        BOOST_LIBRARYDIR = "${pkgs.lib.getLib pkgs.boost}/lib";

        buildInputs = [
          pkgs.meson
          pkgs.ninja
          pkgs.libgit2
          pkgs.pkg-config
          pkgs.boost
        ];
      };
    in
    {
      packages.x86_64-linux.git-recent = git-recent;
      packages.x86_64-linux.default = git-recent;
    };
}
