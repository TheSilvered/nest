@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

IF EXIST x64 RMDIR x64 /S /Q
IF EXIST x86 RMDIR x86 /S /Q

MKDIR x64\nest_libs\font
MKDIR x86\nest_libs\font

:: Main executable
:: ===============

COPY projects\nest\x64\Release\nest.exe x64\nest.exe
COPY projects\nest\Release\nest.exe x86\nest.exe

:: Interpreter DLL
:: ===============

COPY projects\nest\x64\Release\libnest.dll x64\libnest.dll
COPY projects\nest\Release\libnest.dll x86\libnest.dll

:: Standard Library
:: ================

COPY projects\nest\x64\Release\nest_*.dll x64\nest_libs
COPY projects\nest\Release\nest_*.dll x86\nest_libs

COPY ..\..\libs\_nest_files\* x64\nest_libs
COPY ..\..\libs\_nest_files\* x86\nest_libs

FOR %%G IN (x64\nest_libs\*.dll) DO (
    SET fpath=%%G
    SET fpath=!fpath:nest_=std!
    SET fpath=!fpath:stdlibs=nest_libs!
    SET fpath=!fpath:.dll=.cnest!
    MOVE %%G !fpath!
)

FOR %%G IN (x86\nest_libs\*.dll) DO (
    SET fpath=%%G
    SET fpath=!fpath:nest_=std!
    SET fpath=!fpath:stdlibs=nest_libs!
    SET fpath=!fpath:.dll=.cnest!
    MOVE %%G !fpath!
)

:: Udate .nest files
:: =================

IF "%1" == "py" (
    FOR %%G IN (x64\nest_libs\*.nest) DO py ..\update_nest_file.py %%G
    FOR %%G IN (x86\nest_libs\*.nest) DO py ..\update_nest_file.py %%G
) ELSE (
    FOR %%G IN (x64\nest_libs\*.nest) DO nest ..\update_nest_file.nest %%G
    FOR %%G IN (x86\nest_libs\*.nest) DO nest ..\update_nest_file.nest %%G
)

ENDLOCAL
