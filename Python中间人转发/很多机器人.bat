
@echo off
setlocal enabledelayedexpansion

:: Loop to start multiple instances of the Python script
for /L %%i in (1,1,20) do (
    start python �ҷ�����.py  --username test%%i
)

endlocal

