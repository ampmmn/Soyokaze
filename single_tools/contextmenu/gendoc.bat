@echo off
pushd %~dp0

pandoc -c "%~dp0github-markdown.css" -s --embed-resources --toc --standalone -f markdown -t html README.md  -o contextmenu.html

popd
goto :EOF
