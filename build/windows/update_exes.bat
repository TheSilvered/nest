@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

IF NOT EXIST .\x64\nest_libs MKDIR .\x64\nest_libs
IF NOT EXIST .\x86\nest_libs MKDIR .\x86\nest_libs

:: Main executable
:: ===============

COPY projects\nest\x64\Release\nest.exe x64\nest.exe /Y
COPY projects\nest\Release\nest.exe x86\nest.exe /Y

:: Interpreter DLL
:: ===============

COPY projects\nest\x64\Release\libnest.dll x64\libnest.dll /Y
COPY projects\nest\Release\libnest.dll x86\libnest.dll /Y

:: SDL DLLs
:: ========

:: COPY ..\..\libs\nest_gui\SDL2\lib\x64\SDL2.dll x64\SDL2.dll /Y
:: COPY ..\..\libs\nest_gui\SDL2\lib\x86\SDL2.dll x86\SDL2.dll /Y

:: COPY ..\..\libs\nest_gui\SDL2\lib\x64\SDL2_ttf.dll x64\SDL2_ttf.dll /Y
:: COPY ..\..\libs\nest_gui\SDL2\lib\x86\SDL2_ttf.dll x86\SDL2_ttf.dll /Y

:: Standard Library
:: ================

DEL x64\nest_libs\*.dll /Q
DEL x86\nest_libs\*.dll /Q

COPY projects\nest\x64\Release\nest_*.dll x64\nest_libs /Y
COPY projects\nest\Release\nest_*.dll x86\nest_libs /Y

COPY ..\..\libs\_nest_files\* x64\nest_libs /Y
COPY ..\..\libs\_nest_files\* x86\nest_libs /Y

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
    FOR %%G IN (x64\nest_libs\std*.nest) DO py ..\update_nest_file.py %%G
    FOR %%G IN (x86\nest_libs\std*.nest) DO py ..\update_nest_file.py %%G
) ELSE (
    FOR %%G IN (x64\nest_libs\std*.nest) DO nest ..\update_nest_file.nest %%G
    FOR %%G IN (x86\nest_libs\std*.nest) DO nest ..\update_nest_file.nest %%G
)

ENDLOCAL
