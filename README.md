
# Table of Contents

1.  [Description](#org8a2e5b2)
2.  [Dependecies](#org7447265)
    1.  [Windows](#orgf085184)
    2.  [Linux](#orgc741a76)
3.  [Building](#org0ee5d77)
    1.  [Windows](#org8b0743e)
    2.  [Linux](#org99f4cd8)

![img](https://github.com/iliayar/UPA_CAN_FLASH/actions/workflows/main.yml/badge.svg)


<a id="org8a2e5b2"></a>

# Description

Can bus flashing software on top of ISO-14229.


<a id="org7447265"></a>

# Dependecies


<a id="orgf085184"></a>

## Windows

Tested with following setup:

-   MSYS2 MinGW 64
-   Qt 5.12.2
-   MinGW gcc 10.2.0
-   MinGW CMake 3.19.2
-   MinGW make 4.3
-   NSIS


<a id="orgc741a76"></a>

## Linux

Tested with:

-   CMake 3.19.5
-   gcc 10.2.0
-   Qt 5.12.2
-   OPTIONAL Doxygen 1.9.1 for docs


<a id="org0ee5d77"></a>

# Building

-   Clone this repository
-   Create build directory `mkdir build` and cd into it `cd build`


<a id="org8b0743e"></a>

## Windows

-   Generate Makefile `cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..`
-   Compile binaries using `mingw32-make`


<a id="org99f4cd8"></a>

## Linux

-   Generate Makefile `cmake -DCMAKE_BUILD_TYPE=Release ..`
-   Compile binaries and docs `make`

