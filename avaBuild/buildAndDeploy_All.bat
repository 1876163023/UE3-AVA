p4 -c avacomfile sync //ava.dev/...@%1
vcbuild /M4 ava.sln "releaseNet|Win32"
if NOT ERRORLEVEL 0 goto end
vcbuild /M4 ava.sln "releaseLTCG|Win32"
if NOT ERRORLEVEL 0 goto end
..\..\binaries\avagame.com make -unattended -full
if NOT ERRORLEVEL 0 goto end
rd/s/q ..\..\avaGame\Script__
ren ..\..\avaGame\Script Script__
..\..\binaries\avagame.com make -unattended -full -final_release
ren ..\..\avaGame\Script__ Script
if NOT ERRORLEVEL 0 goto end
deploy.pl %1 deployFileList_All.txt
:end
