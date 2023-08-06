pushd %~dp0

set APPNAME=Soyokaze

pandoc -c "%~dp0github-markdown.css" -s --embed-resources --standalone --toc --toc-depth=4 -f markdown -t html --metadata title="%APPNAME% ƒ}ƒjƒ…ƒAƒ‹" %~dp0help.md > %~dp0help.html

popd

