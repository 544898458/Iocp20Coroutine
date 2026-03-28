@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM 启动全部服务器脚本 (Windows版本)
REM 功能：按顺序启动WorldSvr、GameSvr、GateSvr

echo 正在启动幸存者游戏服务器...

REM 创建日志文件夹
if not exist "Log\WorldSvr" mkdir "Log\WorldSvr"
if not exist "Log\GameSvr" mkdir "Log\GameSvr"
if not exist "Log\GateSvr" mkdir "Log\GateSvr"

REM 检查可执行文件是否存在
if not exist "WorldSvr.exe" (
    echo 错误：找不到 WorldSvr.exe 可执行文件
    pause
    exit /b 1
)

if not exist "GameSvr.exe" (
    echo 错误：找不到 GameSvr.exe 可执行文件
    pause
    exit /b 1
)

if not exist "GateSvr.exe" (
    echo 错误：找不到 GateSvr.exe 可执行文件
    pause
    exit /b 1
)

echo 启动 WorldSvr.exe...
start "WorldSvr" WorldSvr.exe

REM 等待2秒
echo 等待2秒...
timeout /t 2 /nobreak >nul

echo 启动 GameSvr.exe 1 ...
start "GameSvr_1" GameSvr.exe 1

echo 启动 GameSvr.exe 2 ...
start "GameSvr_2" GameSvr.exe 2

echo 启动 GameSvr.exe 3 ...
start "GameSvr_3" GameSvr.exe 3

REM 等待6秒
echo 等待6秒...
timeout /t 6 /nobreak >nul

echo 启动 GateSvr.exe...
start "GateSvr" GateSvr.exe

echo.
echo 所有服务器已启动完成！
echo.
echo 停止服务器：关闭各个窗口即可
echo.
pause
