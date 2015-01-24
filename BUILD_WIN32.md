Windows - Visual Studio 2013
============================
#### Prerequisites ####
* Microsoft Visual C++ 2013 Update 1 (the free Express edition will work)
* If you have multiple MSVS installation use MSVS Developer console from target version.
* You can build either Windows 32bit or 64bit binaries.

#### Set up the directory structure####
* Create a base directory for all projects.  I'm putting everything in 
  `D:\LarkX, you can use whatever you like.  In several of the batch files 
  and makefiles, this directory will be referred to as `BITSHARES_ROOT`:
  ```
mkdir D:\LarkX
```

* Clone the LarkX repository
  ```
cd D:\LarkX
git clone https://github.com/larkx/LarkX.git
cd LarkX
git submodule init
git submodule update
cd vendor
git clone https://github.com/InvictusInnovations/leveldb-win.git
```

* Dowload CMake
  
  Download the latest *Win32 Zip* build CMake from 
  http://cmake.org/cmake/resources/software.html (version 2.8.12.2 as of this 
  writing).  Unzip it to your base directory, which will create a directory that
  looks something like `D:\LarkX\cmake-2.8.12.2-win32-x86`.  Rename this
  directory to `D:\LarkX\CMake`.

  If you already have CMake installed elsewhere on your system you can use it, 
  but LarkX has a few batch files that expect it to be in the base
  directory's `CMake` subdirectory, so those scripts would need tweaking.

* Download library dependencies:
   You can get prebuilt packages containing all of the libraries described below built
   by using currently used compiler. Download the appropriate package from one of the links below
   (or both if you plan to build 32bit and 64bit versions).

   32bit version:    http://get.syncad.com/invictus/Bitshare_toolkit_prebuilt_libs-vs2013.7z
   64bit version:    http://get.syncad.com/invictus/Bitshare_toolkit_prebuilt_libs-vs2013-x64.7z

   and should be unpacked into BITSHARES_ROOT directory (ie: D:\LarkX).
   Now the package contains: BerkeleyDB, boost 1.55, OpenSSL 1.0.1g.

 * BerkeleyDB

   LarkX depends on BerkeleyDB 12c Release 1 (12.1.6.0.20).  You can build
   this from source or download our pre-built binaries to speed things up.

 * Boost
 
   LarkX depends on the Boost libraries version 1.55 or later (I assume
   you're using 1.55, the latest as of this writing).  You must build them from
   source if you're building 32bit binaries, but the 64bit binaries include
    prebuilt boost binaries, so no need to build them again for 64bit development).
   * download the latest boost source from http://www.boost.org/users/download/
   * unzip it to the base directory `D:\LarkX`.
   * This will create a directory like `D:\Lark\boost_1_55_0`.
   
 * OpenSSL

   LarkX depends on OpenSSL, and you must build this from source.
    * download the latest OpenSSL source from http://www.openssl.org/source/
    * Untar it to the base directory `D:\LarkX`
    * this will create a directory like `D:\LarkX\openssl-1.0.1g`.

At the end of this, your base directory should look like this (directory names will
be slightly different for the 64bit versions):
```
D:\LarkX
+- BerkeleyDB
+- LarkX
+- boost_1.55
+- CMake
+- OpenSSL
```

#### Build the library dependencies - Skip if you downloaded the prebuilt binary package(s) ####

* Set up environment for building:
  ```
cd D:\LarkX\LarkX
setenv.bat (or setenv_x64.bat for 64bit development)
```

* Build boost libraries (required for 32bit builds only, skip this step for 64bit development):
  ```
cd D:\LarkX\boost
bootstrap.bat
b2.exe toolset=msvc-11.0 variant=debug,release link=static threading=multi runtime-link=shared address-model=32
```
  The file `D:\LarkX\LarkX\libraries\fc\CMakeLists.txt` has the
  `FIND_PACKAGE(Boost ...)`
  command that makes CMake link in Boost.  That file contains the line:
  ```
set(Boost_USE_DEBUG_PYTHON ON)
```
  Edit this file and comment this line out (with a `#`).
  This line  tells CMake to look for a boost library that was built with 
  `b2.exe link=shared python-debugging=on`.  That would cause debug builds to 
  have `-gyd` mangled into their filename.  We don't need python debugging here,
  so we didn't give the `python-debugging` argument to `b2.exe`, and
  that causes our boost debug libraries to have `-gd` mangled into the filename 
  instead.  If this option in `fc\CMakeLists.txt` doesn't match the way you 
  compiled boost, CMake won't be able to find the debug version of the boost 
  libraries, and you'll get some strange errors when you try to run the
  debug version of LarkX.

* Build OpenSSL DLLs
  ```
cd D:\LarkX\openssl-1.0.1g
perl Configure --openssldir=D:\LarkX\OpenSSL VC-WIN32
ms\do_ms.bat
nmake -f ms\ntdll.mak
nmake -f ms\ntdll.mak install
```
  This will create the directory `D:\LarkX\OpenSSL` with the libraries, DLLs,
  and header files.

#### Build project files for LarkX ####

* Run CMake:
  ```
cd D:\LarkX\LarkX
run_cmake.bat (or run_cmake_x64.bat for 64bit development)
```
 This pops up the cmake gui, but if you've used CMake before it will probably be
 showing the wrong data, so fix that:
 * Where is the source code: `D:\LarkX\LarkX
 * Where to build the binaries: `D:\LarkX\bin` (or bin64 for 64bit development).
 
 Then hit **Configure**.  It may ask you to specify a generator for this 
 project; if it does, choose **Visual Studio 12 2013** or **Visual Studio 12 2013 Win64** for 64 bit builds and select **Use default 
 native compilers**.  Look through the output and fix any errors.  Then 
 hit **Generate**.

Please note that if you are targeting Windows XP platform you need to add the cache entry "CMAKE_GENERATOR_TOOLSET" to contain the "v120_xp" value before configuring.

Or you can run Cmake in command line:
> cd build
> cmake -G "Visual Studio 12" -T "v120_xp" -DINCLUDE_QT_WALLET ..\LarkX

#### Build LarkX ####
* Set up environment for building (Skip if you already did this when building library dependencies):

  ```
cd D:\LarkX\LarkX
setenv.bat (or setenv_x64.bat for 64bit development)
```

* Launch *Visual Studio* and load `D:\LarkX\bin\LarkX.sln` bin64 directory for 64 bit builds.
* Set Active Configuration to RelWithDebInfo, ensure Active Solution platform is x86 for 32 bit platforms and x64 for 64 bit builds

* *Build Solution*

 This will build the client executable and various unit tests.

Or you can build the `INSTALL` target in Visual Studio which will
copy all of the necessary files into your `D:\LarkX\install`
directory, then copy all of those files to the `bin` directory.
