:: Main executable
copy ..\x64\Release\nest.exe x64\nest.exe /y
copy ..\Release\nest.exe x86\nest.exe /y

:: Standard Library
:: ================

:: File system
copy ..\x64\Release\nest_fs.dll x64\nest_libs\stdfs.dll /y
copy ..\Release\nest_fs.dll x86\nest_libs\stdfs.dll /y
copy ..\nest_libs\nest_fs\stdfs.nest x64\nest_libs\stdfs.nest /y
copy ..\nest_libs\nest_fs\stdfs.nest x86\nest_libs\stdfs.nest /y
:: Input/Output
copy ..\x64\Release\nest_io.dll x64\nest_libs\stdio.dll /y
copy ..\Release\nest_io.dll x86\nest_libs\stdio.dll /y
copy ..\nest_libs\nest_io\stdio.nest x64\nest_libs\stdio.nest /y
copy ..\nest_libs\nest_io\stdio.nest x86\nest_libs\stdio.nest /y
:: Random
copy ..\x64\Release\nest_rand.dll x64\nest_libs\stdrand.dll /y
copy ..\Release\nest_rand.dll x86\nest_libs\stdrand.dll /y
copy ..\nest_libs\nest_rand\stdrand.nest x64\nest_libs\stdrand.nest /y
copy ..\nest_libs\nest_rand\stdrand.nest x86\nest_libs\stdrand.nest /y
:: String utitlities
copy ..\x64\Release\nest_sutil.dll x64\nest_libs\stdsutil.dll /y
copy ..\Release\nest_sutil.dll x86\nest_libs\stdsutil.dll /y
copy ..\nest_libs\nest_sutil\stdsutil.nest x64\nest_libs\stdsutil.nest /y
copy ..\nest_libs\nest_sutil\stdsutil.nest x86\nest_libs\stdsutil.nest /y
:: Time and date
copy ..\x64\Release\nest_time.dll x64\nest_libs\stdtime.dll /y
copy ..\Release\nest_time.dll x86\nest_libs\stdtime.dll /y
copy ..\nest_libs\nest_time\stdtime.nest x64\nest_libs\stdtime.nest /y
copy ..\nest_libs\nest_time\stdtime.nest x86\nest_libs\stdtime.nest /y
:: Iteration utilities
copy ..\x64\Release\nest_itutil.dll x64\nest_libs\stditutil.dll /y
copy ..\Release\nest_itutil.dll x86\nest_libs\stditutil.dll /y
copy ..\nest_libs\nest_itutil\stditutil.nest x64\nest_libs\stditutil.nest /y
copy ..\nest_libs\nest_itutil\stditutil.nest x86\nest_libs\stditutil.nest /y
:: Math
copy ..\x64\Release\nest_math.dll x64\nest_libs\stdmath.dll /y
copy ..\Release\nest_math.dll x86\nest_libs\stdmath.dll /y
copy ..\nest_libs\nest_math\stdmath.nest x64\nest_libs\stdmath.nest /y
copy ..\nest_libs\nest_math\stdmath.nest x86\nest_libs\stdmath.nest /y
:: Sequence utilities
copy ..\x64\Release\nest_sequtil.dll x64\nest_libs\stdsequtil.dll /y
copy ..\Release\nest_sequtil.dll x86\nest_libs\stdsequtil.dll /y
copy ..\nest_libs\nest_sequtil\stdsequtil.nest x64\nest_libs\stdsequtil.nest /y
copy ..\nest_libs\nest_sequtil\stdsequtil.nest x86\nest_libs\stdsequtil.nest /y
:: System
copy ..\x64\Release\nest_sys.dll x64\nest_libs\stdsys.dll /y
copy ..\Release\nest_sys.dll x86\nest_libs\stdsys.dll /y
copy ..\nest_libs\nest_sys\stdsys.nest x64\nest_libs\stdsys.nest /y
copy ..\nest_libs\nest_sys\stdsys.nest x86\nest_libs\stdsys.nest /y

:: Udate .nest files
py update_nest_file.py x64\nest_libs\stdfs.nest
py update_nest_file.py x86\nest_libs\stdfs.nest
py update_nest_file.py x64\nest_libs\stdio.nest
py update_nest_file.py x86\nest_libs\stdio.nest
py update_nest_file.py x64\nest_libs\stdrand.nest
py update_nest_file.py x86\nest_libs\stdrand.nest
py update_nest_file.py x64\nest_libs\stdsutil.nest
py update_nest_file.py x86\nest_libs\stdsutil.nest
py update_nest_file.py x64\nest_libs\stdtime.nest
py update_nest_file.py x86\nest_libs\stdtime.nest
py update_nest_file.py x64\nest_libs\stditutil.nest
py update_nest_file.py x86\nest_libs\stditutil.nest
py update_nest_file.py x64\nest_libs\stdmath.nest
py update_nest_file.py x86\nest_libs\stdmath.nest
py update_nest_file.py x64\nest_libs\stdsequtil.nest
py update_nest_file.py x86\nest_libs\stdsequtil.nest
py update_nest_file.py x64\nest_libs\stdsys.nest
py update_nest_file.py x86\nest_libs\stdsys.nest
