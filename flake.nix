{
  description = "A C++ wrapper for the MariaDB C Connector";
  inputs = {
    nixpkgs.url = "nixpkgs/nixos-26.05";
  };
  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };

      mariadb-connector-c = pkgs.stdenv.mkDerivation {
        pname = "mariadb-connector-c";
        version = "3.4";
        src = ./vendor/mariadb-connector-c;

        preConfigure = ''
          sed -i 's/MYSQL_TIME tm;/MYSQL_TIME tm = {0};/' libmariadb/mariadb_rpl.c
        '';

        cmakeFlags = [
          "-DLIBMARIADB=ON"
        ];
        nativeBuildInputs = [ pkgs.cmake ];
        buildInputs = [
          pkgs.openssl
        ];
      };
    in
    {
      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "mdbcxx";
        version = "0.1.0";
        src = ./.;

        nativeBuildInputs = with pkgs; [
          cmake
          pkg-config
        ];
        buildInputs = [ mariadb-connector-c ];
      };

      devShells.${system}.default = pkgs.mkShell {
        nativeBuildInputs = with pkgs; [
          pkg-config
          cmake
          clang-tools
          gf
        ];
        buildInputs = with pkgs; [
          catch2_3
          mariadb-connector-c
        ];
      };
    };
}
