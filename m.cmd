SETLOCAL
cls
IF NOT DEFINED ZAR_TOOLCHAIN SET ZAR_TOOLCHAIN=msvc
IF NOT DEFINED ZAR_BUILDDIR SET ZAR_BUILDDIR=obj

CALL .\ninja\make-zlib.cmd

DEL build.ninja
REM copy in binary mode to avoid a trailing EOF ^Z.
COPY /B /Y ninja\global.ninja + ninja\%ZAR_TOOLCHAIN%.ninja + ninja\rules.ninja build.ninja

ninja.exe %*
ENDLOCAL
