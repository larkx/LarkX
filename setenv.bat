@echo off
set BITSHARES_ROOT=%~dp0..\
echo Using %BITSHARES_ROOT% as LarkX root directory

set OPENSSL_ROOT=%BITSHARES_ROOT%\OpenSSL
set OPENSSL_ROOT_DIR=%OPENSSL_ROOT%
set OPENSSL_INCLUDE_DIR=%OPENSSL_ROOT%\include
set DBROOTDIR=%BITSHARES_ROOT%\BerkeleyDB
set QTDIR=%BITSHARES_ROOT%\qt\5.3\msvc2013
set ICUROOT=%BITSHARES_ROOT%\ICU
set BOOST_ROOT=%BITSHARES_ROOT%\boost_1.55
set TCL_ROOT=%BITSHARES_ROOT%\tcl
set NPM_INSTALL_PREFIX=%BITSHARES_ROOT%\npm

set PATH=%NPM_INSTALL_PREFIX%;"%APPDATA%"\npm;%QTDIR%\bin;%ICUROOT%\bin;%ICUROOT%\lib;%BITSHARES_ROOT%\bin;%TCL_ROOT%\bin;%BITSHARES_ROOT%\Cmake\bin;%BOOST_ROOT%\stage\lib;%PATH%
set CMAKE_PREFIX_PATH=%QTDIR%
set QT_QPA_PLATFORM_PLUGIN_PATH=%QTDIR%\plugins\platforms

echo Setting up VS2013 environment...
call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat"
