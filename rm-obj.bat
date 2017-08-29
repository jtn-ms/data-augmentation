del /Q /S *.aps
del /Q /S *.sdf
del /Q /S *.suo
del /Q /S *.pdb
del /Q /S *.ilk
del /Q /S *.exp
del /Q /S *.error

rmdir /Q /S ipch
rmdir /Q /S VAMain\Win32
rmdir /Q /S VAEngine\Win32
rmdir /Q /S VAEngine\GeneratedFiles
rmdir /Q /S *.pdb

pause
