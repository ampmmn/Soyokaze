pushd %~dp0

if not exist html mkdir html

call :CONVERT help
REM call :CONVERT window-input
REM call :CONVERT top
REM call :CONVERT faq

popd
goto :EOF

:CONVERT
  set INPUT=%1
	set OUTPUT=%INPUT%
	shift
  pandoc -c "%~dp0github-markdown.css" -s --embed-resources --standalone --toc --toc-depth=4 -f markdown -t html  %~dp0%INPUT%.md -o %OUTPUT%.html
REM  pandoc -c "%~dp0github-markdown.css" -s --embed-resources --standalone --toc --toc-depth=4 -f markdown -t html  %~dp0%INPUT%.md --filter=pandoc-include --filter=%~dp0convertlink.py -o html\%OUTPUT%.html


