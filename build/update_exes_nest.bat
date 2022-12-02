:: Main executable
copy ..\x64\Release\nest.exe x64\nest.exe /y
copy ..\Release\nest.exe x86\nest.exe /y

:: Standard Library
:: ================

:: Error
copy ..\x64\Release\nest_err.dll x64\nest_libs\stderr.dll /y
copy ..\Release\nest_err.dll x86\nest_libs\stderr.dll /y
copy ..\libs\_nest_files\stderr.nest x64\nest_libs\stderr.nest /y
copy ..\libs\_nest_files\stderr.nest x86\nest_libs\stderr.nest /y
:: File system
copy ..\x64\Release\nest_fs.dll x64\nest_libs\stdfs.dll /y
copy ..\Release\nest_fs.dll x86\nest_libs\stdfs.dll /y
copy ..\libs\_nest_files\stdfs.nest x64\nest_libs\stdfs.nest /y
copy ..\libs\_nest_files\stdfs.nest x86\nest_libs\stdfs.nest /y
:: Input/Output
copy ..\x64\Release\nest_io.dll x64\nest_libs\stdio.dll /y
copy ..\Release\nest_io.dll x86\nest_libs\stdio.dll /y
copy ..\libs\_nest_files\stdio.nest x64\nest_libs\stdio.nest /y
copy ..\libs\_nest_files\stdio.nest x86\nest_libs\stdio.nest /y
:: Random
copy ..\x64\Release\nest_rand.dll x64\nest_libs\stdrand.dll /y
copy ..\Release\nest_rand.dll x86\nest_libs\stdrand.dll /y
copy ..\libs\_nest_files\stdrand.nest x64\nest_libs\stdrand.nest /y
copy ..\libs\_nest_files\stdrand.nest x86\nest_libs\stdrand.nest /y
:: String utitlities
copy ..\x64\Release\nest_sutil.dll x64\nest_libs\stdsutil.dll /y
copy ..\Release\nest_sutil.dll x86\nest_libs\stdsutil.dll /y
copy ..\libs\_nest_files\stdsutil.nest x64\nest_libs\stdsutil.nest /y
copy ..\libs\_nest_files\stdsutil.nest x86\nest_libs\stdsutil.nest /y
:: Time and date
copy ..\x64\Release\nest_time.dll x64\nest_libs\stdtime.dll /y
copy ..\Release\nest_time.dll x86\nest_libs\stdtime.dll /y
copy ..\libs\_nest_files\stdtime.nest x64\nest_libs\stdtime.nest /y
copy ..\libs\_nest_files\stdtime.nest x86\nest_libs\stdtime.nest /y
:: Iteration utilities
copy ..\x64\Release\nest_itutil.dll x64\nest_libs\stditutil.dll /y
copy ..\Release\nest_itutil.dll x86\nest_libs\stditutil.dll /y
copy ..\libs\_nest_files\stditutil.nest x64\nest_libs\stditutil.nest /y
copy ..\libs\_nest_files\stditutil.nest x86\nest_libs\stditutil.nest /y
:: Math
copy ..\x64\Release\nest_math.dll x64\nest_libs\stdmath.dll /y
copy ..\Release\nest_math.dll x86\nest_libs\stdmath.dll /y
copy ..\libs\_nest_files\stdmath.nest x64\nest_libs\stdmath.nest /y
copy ..\libs\_nest_files\stdmath.nest x86\nest_libs\stdmath.nest /y
:: Sequence utilities
copy ..\x64\Release\nest_sequtil.dll x64\nest_libs\stdsequtil.dll /y
copy ..\Release\nest_sequtil.dll x86\nest_libs\stdsequtil.dll /y
copy ..\libs\_nest_files\stdsequtil.nest x64\nest_libs\stdsequtil.nest /y
copy ..\libs\_nest_files\stdsequtil.nest x86\nest_libs\stdsequtil.nest /y
:: System
copy ..\x64\Release\nest_sys.dll x64\nest_libs\stdsys.dll /y
copy ..\Release\nest_sys.dll x86\nest_libs\stdsys.dll /y
copy ..\libs\_nest_files\stdsys.nest x64\nest_libs\stdsys.nest /y
copy ..\libs\_nest_files\stdsys.nest x86\nest_libs\stdsys.nest /y
:: Coroutines
copy ..\x64\Release\nest_co.dll x64\nest_libs\stdco.dll /y
copy ..\Release\nest_co.dll x86\nest_libs\stdco.dll /y
copy ..\libs\_nest_files\stdco.nest x64\nest_libs\stdco.nest /y
copy ..\libs\_nest_files\stdco.nest x86\nest_libs\stdco.nest /y

:: Udate .nest files
nest update_nest_file.nest x64\nest_libs\stdco.nest
nest update_nest_file.nest x86\nest_libs\stdco.nest
nest update_nest_file.nest x64\nest_libs\stderr.nest
nest update_nest_file.nest x86\nest_libs\stderr.nest
nest update_nest_file.nest x64\nest_libs\stdfs.nest
nest update_nest_file.nest x86\nest_libs\stdfs.nest
nest update_nest_file.nest x64\nest_libs\stdio.nest
nest update_nest_file.nest x86\nest_libs\stdio.nest
nest update_nest_file.nest x64\nest_libs\stdrand.nest
nest update_nest_file.nest x86\nest_libs\stdrand.nest
nest update_nest_file.nest x64\nest_libs\stdsutil.nest
nest update_nest_file.nest x86\nest_libs\stdsutil.nest
nest update_nest_file.nest x64\nest_libs\stdtime.nest
nest update_nest_file.nest x86\nest_libs\stdtime.nest
nest update_nest_file.nest x64\nest_libs\stditutil.nest
nest update_nest_file.nest x86\nest_libs\stditutil.nest
nest update_nest_file.nest x64\nest_libs\stdmath.nest
nest update_nest_file.nest x86\nest_libs\stdmath.nest
nest update_nest_file.nest x64\nest_libs\stdsequtil.nest
nest update_nest_file.nest x86\nest_libs\stdsequtil.nest
nest update_nest_file.nest x64\nest_libs\stdsys.nest
nest update_nest_file.nest x86\nest_libs\stdsys.nest
