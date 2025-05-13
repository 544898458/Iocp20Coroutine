
@echo off
setlocal enabledelayedexpansion

:: Loop to start multiple instances of the Python script
for /L %%i in (11,1,15) do (
    :: 间隔1秒
    timeout /t 1 /nobreak >nul
    start python 乱发测试.py  --username test%%i
)

endlocal

