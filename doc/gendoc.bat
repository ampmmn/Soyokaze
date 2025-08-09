pushd %~dp0

set SPHINXOPTS=-D language=ja

pushd "%~dp0manuals\"

pushd source
rmdir image
mklink /j image ..\..\..\image
popd

call make.bat html

popd
goto :EOF

