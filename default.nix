{
  pkgs ? import <nixpkgs> { }
}:
let
  stdenv = pkgs.stdenv;
  wrapper = pkgs.libsForQt512.qt5.wrapQtAppsHook;
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    can-utils
    wrapper

    (python39.withPackages (pypkgs: with pypkgs; [
      can
      setuptools
    ]))
    nodePackages.pyright
  ];

  buildInputs = with pkgs; [
    libsForQt512.qt5.qtbase
    libsForQt512.qt5.qtserialbus
  ];
}
