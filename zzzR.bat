@REM this simply reflects Debug Environment settings in VS
@echo off
set PATH=%ORACLE_HOME1%\oci\lib\msvc\vc14;%ORACLE_HOME1%\bin;%CUDA_PATH%\bin
@REM add standard paths
set PATH=.;%PATH%;%windir%;%windir%\system32
x64\release\client %*

