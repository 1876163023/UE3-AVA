p4 -c avacomfile sync //ava.dev/...@%1
w:buildconsole ava.sln /cfg=ReleaseLTCG
ren ..\..\avaGame\Script Script__
..\..\binaries\avagame.com make -unattended -full -final_release
ren ..\..\avaGame\Script__ Script
deploy.pl %1 deployFileList_LTCG.txt
