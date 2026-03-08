@echo off
chcp 65001 >nul
echo ============================================
echo   智能安防报警系统 - 上传到 GitHub
echo ============================================
echo.

cd /d "D:\BaiduNetdiskDownload\智能安防报警系统"

echo [1/5] 初始化 Git 仓库...
git init
echo.

echo [2/5] 配置 Git（防止中文乱码）...
git config core.quotepath false
echo.

echo [3/5] 添加所有文件...
git add .
echo.

echo [4/5] 提交代码...
git commit -m "feat: 智能安防报警系统 - 初始提交"
echo.

echo [5/5] 关联 GitHub 并推送...
echo.
echo !! 请先在 GitHub 上创建仓库 !!
echo    1. 打开浏览器访问: https://github.com/new
echo    2. Repository name 填写: smart-security-alarm
echo    3. 不要勾选 "Add a README file"
echo    4. 点击 "Create repository"
echo    5. 创建完成后回来按任意键继续
echo.
pause

git remote add origin https://github.com/rua103/smart-security-alarm.git
git branch -M main
git push -u origin main

echo.
echo ============================================
echo   上传完成！
echo   仓库地址: https://github.com/rua103/smart-security-alarm
echo ============================================
pause

