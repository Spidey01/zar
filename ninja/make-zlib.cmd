
@IF NOT DEFINED ZAR_BUILDDIR (
    @ECHO Must define ZAR_BUILDDIR to use %0
    GOTO :eof
)
@IF NOT DEFINED ZAR_TOOLCHAIN (
    @ECHO Must define ZAR_TOOLCHAIN to use %0
    GOTO :eof
)

IF EXIST %ZAR_BUILDDIR%\zlib.lib GOTO :eof

git submodule init
git submodule update

MKDIR %ZAR_BUILDDIR%

robocopy /E zlib %ZAR_BUILDDIR%\zlib
PUSHD %ZAR_BUILDDIR%\zlib
nmake /F .\win32\Makefile.msc zlib.lib
COPY /B /Y zlib.lib ..\zlib.lib
POPD

