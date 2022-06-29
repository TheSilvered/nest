:: Main executable
copy ..\x64\Release\nest.exe nest.exe /y

:: Standard Library
:: ================

:: File system
copy ..\nest_libs\nest_fs\x64\Release\nest_fs.dll nest_libs\stdfs.dll /y
copy ..\nest_libs\nest_fs\stdfs.nest nest_libs\stdfs.nest /y
:: Input/Output
copy ..\nest_libs\nest_io\x64\Release\nest_io.dll nest_libs\stdio.dll /y
copy ..\nest_libs\nest_io\stdio.nest nest_libs\stdio.nest /y
:: Random
copy ..\nest_libs\nest_rand\x64\Release\nest_rand.dll nest_libs\stdrand.dll /y
copy ..\nest_libs\nest_rand\stdrand.nest nest_libs\stdrand.nest /y
:: String utitlities
copy ..\nest_libs\nest_sutil\x64\Release\nest_sutil.dll nest_libs\stdsutil.dll /y
copy ..\nest_libs\nest_sutil\stdsutil.nest nest_libs\stdsutil.nest /y
:: Time and date
copy ..\nest_libs\nest_time\x64\Release\nest_time.dll nest_libs\stdtime.dll /y
copy ..\nest_libs\nest_time\stdtime.nest nest_libs\stdtime.nest /y

:: Udate .nest files
nest update_nest_file.nest nest_libs\stdfs.nest
nest update_nest_file.nest nest_libs\stdio.nest
nest update_nest_file.nest nest_libs\stdrand.nest
nest update_nest_file.nest nest_libs\stdsutil.nest
nest update_nest_file.nest nest_libs\stdtime.nest