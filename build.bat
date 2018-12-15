@call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsMSBuildCmd.bat"
msbuild zzz.sln /p:Configuration=%1 /p:Platform=x64 /verbosity:quiet