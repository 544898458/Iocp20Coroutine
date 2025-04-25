
@echo off
setlocal enabledelayedexpansion

:: Loop to start multiple instances of the Python script
for /L %%i in (1,1,5) do (
    start python ÂÒ·¢²âÊÔ.py  --username test%%i
)

endlocal

