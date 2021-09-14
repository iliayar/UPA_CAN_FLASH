{
  pkgs ? import <nixpkgs> { }
}:
let
  qt5 = pkgs.libsForQt512.qt5;
  stdenv = pkgs.stdenv;
  wrapper = pkgs.libsForQt512.qt5.wrapQtAppsHook;
  runApp = pkgs.writeShellScriptBin "runApp" ''
        rm -Rf main/.UPA_CAN_FLASH*
        make -j8 && \
        main/UPA_CAN_FLASH
  '';
in
pkgs.mkShell {
  src = ./.;

  nativeBuildInputs = with pkgs; [
    cmake
    can-utils
    wrapper
    gdb
    runApp

    (python39.withPackages (pypkgs: with pypkgs; [
      can
      setuptools
    ]))
    nodePackages.pyright
  ];

  buildInputs = with pkgs; [
    qt5.qtbase
    qt5.qtserialbus
  ];

  QT_QPA_PLATFORM_PLUGIN_PATH="${qt5.qtbase.bin}/lib/qt-${qt5.qtbase.version}/plugins";

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
