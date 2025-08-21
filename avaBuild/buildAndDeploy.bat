p4 -c avacomfile sync //ava.dev/...@%1
vcbuild /M4 ava.sln "releaseNet|Win32"
if NOT ERRORLEVEL 0 goto end

..\..\binaries\avagame.com make -unattended -full

if NOT ERRORLEVEL 0 goto end
deploy.pl %1 deployFileList.txt

:end
