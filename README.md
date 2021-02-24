
# Table of Contents

1.  [Description](#orgb9ac75a)
2.  [Dependecies](#orga100cf1)
    1.  [Windows](#org60736eb)
    2.  [Linux](#org5f752a6)
3.  [Building](#orgbe61656)
    1.  [Windows](#org477a72f)
    2.  [Linux](#org954e007)


<a id="orgb9ac75a"></a>

# Description

Can bus flashing software on top of ISO-14229.


<a id="orga100cf1"></a>

# Dependecies


<a id="org60736eb"></a>

## Windows

Tested with following setup:

-   MSYS2 MinGW 64
-   Qt 5.12.2
-   MinGW gcc 10.2.0
-   MinGW CMake 3.19.2
-   MinGW make 4.3
-   NSIS


<a id="org5f752a6"></a>

## Linux

Tested with:

-   CMake 3.19.5
-   gcc 10.2.0
-   Qt 5.12.2
-   OPTIONAL Doxygen 1.9.1 for docs


<a id="orgbe61656"></a>

# Building

-   Clone this repository
-   Create build directory `mkdir build` and cd into it `cd build`


<a id="org477a72f"></a>

## Windows

-   Generate Makefile `cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..`
-   Compile binaries using `mingw32-make`


<a id="org954e007"></a>

## Linux

-   Generate Makefile `cmake -DCMAKE_BUILD_TYPE=Release ..`
-   Compile binaries and docs `make`

