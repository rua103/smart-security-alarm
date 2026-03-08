@echo off
chcp 65001 >nul
echo 正在复制项目模板...
echo.

set "SOURCE=传感器模块+STM32套件资料\传感器模块+STM32套件资料\3mm双色LED模块\测试程序"
set "DEST=测试程序"

if not exist "%DEST%" (
    mkdir "%DEST%"
)

xcopy "%SOURCE%\*" "%DEST%\" /E /I /Y /Q

if %ERRORLEVEL% EQU 0 (
    echo 项目模板复制成功！
    echo.
) else (
    echo 复制过程中出现错误，请检查路径是否正确。
    pause
    exit /b 1
)

echo 正在替换main.c文件...
copy /Y "User\main.c" "%DEST%\User\main.c" >nul

if %ERRORLEVEL% EQU 0 (
    echo main.c文件替换成功！
    echo.
    echo 完成！现在可以打开 %DEST%\Project.uvprojx 开始编译了。
) else (
    echo main.c文件替换失败，请手动复制。
)

echo.
pause

