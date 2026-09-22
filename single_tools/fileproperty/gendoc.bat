pushd %~dp0

pandoc -c "%~dp0github-markdown.css" -s --embed-resources --toc --standalone -f markdown -t html README.md  -o fileproperty.html

popd
goto :EOF

