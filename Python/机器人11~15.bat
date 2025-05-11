
@echo off
setlocal enabledelayedexpansion

:: Loop to start multiple instances of the Python script
for /L %%i in (11,1,15) do (
    :: ¼ä¸ô1Ãë
    timeout /t 1 /nobreak >nul
    start python ÂÒ·¢²âÊÔ.py  --username test%%i
)

endlocal

