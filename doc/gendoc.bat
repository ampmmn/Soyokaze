pushd %~dp0

pandoc -c "%~dp0github-markdown.css" -s --embed-resources --standalone --toc --toc-depth=4 -f markdown -t html  %~dp0help.md > %~dp0help.html

popd

