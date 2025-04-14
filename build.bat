@echo off
echo 正在构建代码可视化工具...

REM 检查Qt环境
where qmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo 错误: 未找到qmake命令
    echo 请确保已安装Qt并将Qt的bin目录添加到PATH环境变量中
    echo 例如: C:\Qt\5.15.2\msvc2019_64\bin
    pause
    exit /b 1
)

REM 执行qmake生成Makefile
qmake -makefile

REM 执行编译
if %ERRORLEVEL% equ 0 (
    echo qmake成功，正在编译...
    nmake
    if %ERRORLEVEL% equ 0 (
        echo 编译成功！
    ) else (
        echo 编译失败！
        pause
        exit /b 1
    )
) else (
    echo qmake失败！
    pause
    exit /b 1
)

echo 构建完成。
echo 你可以运行 CodeVisualizer.exe 启动程序。

pause 