
@echo off
setlocal enabledelayedexpansion

:: Loop to start multiple instances of the Python script
for /L %%i in (11,1,15) do (
    :: ���1��
    timeout /t 1 /nobreak >nul
    start python �ҷ�����.py  --username test%%i
)

endlocal

