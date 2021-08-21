{
  pkgs ? import <nixpkgs> { }
}:
let
  stdenv = pkgs.stdenv;
in
rec {
  UPA_CAN_FLASH = stdenv.mkDerivation {
    name = "UPA_CAN_FLASH";

    nativeBuildInputs = with pkgs; [
      cmake
      libsForQt512.qt5.wrapQtAppsHook
    ];

    buildInputs = with pkgs; [
      libsForQt512.qt5.qtbase
      libsForQt512.qt5.qtserialbus
    ];

    buildPhase = ''
      make -j8
      wrapQtApp main/UPA_CAN_FLASH
    '';

    checkPhase = ''
      ctest
    '';

    shellHook = ''
      echo "UPA_CAN_FLASH Project"
      mkdir build
      cd build
      cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_BUILD_TYPE=Debug ..
    '';
  };
}
