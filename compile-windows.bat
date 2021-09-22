@echo off
echo compile-windows.bat can auto compile all windows solutions.
echo ==========================================================
SETLOCAL
del *.zip
IF "%1"=="32" (set Machine=x86) else (set Machine=x64)
IF "%4"=="2017" (set platformTool=2017) else (set platformTool=2013)
set ProjName=%2
set Config=%3
set QTDIR=%5
set mediaType=%6
set task_tag=%7
set demo_branch=%8
set language=%9

echo QTDIR:%QTDIR%
echo Machine: %Machine%
echo ProjName: %ProjName%
echo Config: %Config%
echo platformTool: %platformTool%
echo demo_branch: %demo_branch%
echo mediaType: %mediaType%
echo build_no: %task_tag%
echo language: %language%

set Local_Path=%~dp0%
echo LocalPath: %Local_Path%
cd %Local_Path%
set QtMsBuild=%LOCALAPPDATA%\QtMsBuild
if %platformTool% == 2017 (
    if %Machine% == x64 (
        echo "vs2017-x64 compile ============================"
        call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" %Machine%
        msbuild %ProjName% /t:Rebuild /p:Configuration=%Config% /p:Platform=%Machine%
) else if %Machine% == x86 (
        echo "vs2017-x86 compile ============================"
        call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" %Machine%
        msbuild %ProjName% /t:Rebuild /p:Configuration=%Config% /p:Platform=%Machine% /p:SubsystemVersion=5.1
		cd %Config% && call %QTDIR%/bin/windeployqt.exe .
		
)
) else (
    if %Machine% == x64 (
    echo "vs2013-x64 compile ============================"
    call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %Machine"%
    msbuild %ProjName% /t:Rebuild /p:Configuration=%Config% /p:Platform=%Machine%
) else if %Machine% == x86 (
    echo "vs2013-x86 compile ============================"
    call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %Machine"%
    msbuild %ProjName% /t:Rebuild /p:Configuration=%Config% /p:Platform=%Machine% /p:SubsystemVersion=5.1
)
)

cd %Local_Path%
if %language%==0 (
    goto chinese
) else (
    goto english
)

:chinese
if "%time:~0,2%" lss "10" (set hh=0%time:~1,1%) else (set hh=%time:~0,2%)
echo hh:%hh%
set cdata=%date:~0,4%%date:~5,2%%date:~8,2%_%hh%%time:~3,2%%time:~6,2%
goto sdkcompile

:english
if "%time:~0,2%" lss "10" (set hh=0%time:~1,1%) else (set hh=%time:~0,2%)
echo hh:%hh%
set cdata=%date:~6,4%%date:~0,2%%date:~3,2%_%hh%%time:~3,2%%time:~6,2%
goto sdkcompile


:sdkcompile
set build_no=
echo "sdkcompile build_no: " %build_no%
7z.exe a -tzip wayang_for_windows_agoraDualTeacher_(%Machine%)_%mediaType%_v%demo_branch%_%cdata%.zip %Config%
echo sdk=wayang_for_windows_agoraDualTeacher_(%Machine%)_%mediaType%_v%demo_branch%_%cdata%.zip > propsfile
exit

:urlcompile
set build_no=%RTC_SDK_URL:~-8,4%
echo "urlcompile build_no: " %build_no%
7z.exe a -tzip wayang_for_windows_agoraDualTeacher_(%Machine%)_%mediaType%_v%demo_branch%_%cdata%.zip %Config%
echo wayang_for_windows_agoraDualTeacher_(%Machine%)_%mediaType%_v%demo_branch%_%cdata%.zip > propsfile
exit

