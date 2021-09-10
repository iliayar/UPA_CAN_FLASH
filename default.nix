{
  pkgs ? import <nixpkgs> { }
}:
let
  stdenv = pkgs.stdenv;
  wrapper = pkgs.libsForQt512.qt5.wrapQtAppsHook;
in
pkgs.mkShell {
  src = ./.;

  nativeBuildInputs = with pkgs; [
    cmake
    can-utils
    wrapper
    gdb

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

  buildPhase = ''
      make -j8
    '';

  checkPhase = ''
      ctest
    '';

  shellHook = ''
      echo "UPA_CAN_FLASH Project"
      [ ! -d build ] && mkdir build
      cd build
      cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=Debug ..
    '';

  # TODO: Use only cmake configuration for installing
  installPhase = ''
      mkdir -p $out/bin
      cp main/UPA_CAN_FLASH $out/bin
      wrapQtApp $out/bin/UPA_CAN_FLASH
    '';
}
