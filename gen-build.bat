cd /d %~dp0
chcp 65001
call gradlew.bat :sbe-tool:checkstyleMain
pause
