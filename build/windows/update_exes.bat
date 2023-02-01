@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

IF NOT EXIST .\x64\nest_libs MKDIR .\x64\nest_libs
IF NOT EXIST .\x86\nest_libs MKDIR .\x86\nest_libs

:: Main executable
:: ===============

COPY projects\nest\x64\Release\nest.exe x64\nest.exe /Y
COPY projects\nest\Release\nest.exe x86\nest.exe /Y

:: Standard Library
:: ================

DEL x64\nest_libs\*.dll /Q
DEL x86\nest_libs\*.dll /Q

COPY projects\nest\x64\Release\nest_*.dll x64\nest_libs /Y
COPY projects\nest\Release\nest_*.dll x86\nest_libs /Y

COPY ..\..\libs\_nest_files\std*.nest x64\nest_libs /Y
COPY ..\..\libs\_nest_files\std*.nest x86\nest_libs /Y

FOR %%G IN (x64\nest_libs\*.dll) DO (
    SET fpath=%%G
    SET fpath=!fpath:nest_=std!
    SET fpath=!fpath:stdlibs=nest_libs!
    MOVE %%G !fpath!
)

FOR %%G IN (x86\nest_libs\*.dll) DO (
    SET fpath=%%G
    SET fpath=!fpath:nest_=std!
    SET fpath=!fpath:stdlibs=nest_libs!
    MOVE %%G !fpath!
)

:: Udate .nest files
:: =================

IF "%1" == "py" (
    FOR %%G IN (x64\nest_libs\std*.nest) DO py ..\update_nest_file.py %%G dll
    FOR %%G IN (x86\nest_libs\std*.nest) DO py ..\update_nest_file.py %%G dll
) ELSE (
    FOR %%G IN (x64\nest_libs\std*.nest) DO nest ..\update_nest_file.nest %%G dll
    FOR %%G IN (x86\nest_libs\std*.nest) DO nest ..\update_nest_file.nest %%G dll
)

ENDLOCAL
