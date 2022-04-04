@echo off
git pull
git add .
git status
set /p msg="Enter commit message then press enter: "
git commit -m "%msg%"
git push
pause