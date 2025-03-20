@echo off
REM ユニットテストを実行した結果を使ってカバレッジレポートを生成する
REM 要OpenCppCoverage https://github.com/OpenCppCoverage/OpenCppCoverage

WHERE msbuild > NUL 2> NUL
if errorlevel 1 call vcvars64.bat

WHERE OpenCppCoverage > NUL 2> NUL
if errorlevel 1 exit OpenCppCoverage is required.&& exit /b 1

if not exist %~dp0..\dist\x64\UnitTest mkdir %~dp0..\dist\x64\UnitTest
pushd %~dp0..\dist\x64\UnitTest

msbuild %~dp0..\src/Soyokaze.sln  /t:build /p:configuration=UnitTest /p:Platform="x64" -m

if errorlevel 1 pause && exit /b 1

OpenCppCoverage --source Soyokaze\src\* SoyokazeUnitTest.exe --export_type html:%~dp0..\coverage_report

popd

pause
