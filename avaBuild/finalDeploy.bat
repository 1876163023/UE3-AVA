c:

cd deploy

REM ÀÏ´Ü ½ÌÅ©
p4 -c avacomdeploy sync //ava.dev/...


REM WRANGLE CONTENT
rem 	p4 -c avacomsrc sync //ava.dev/...
rem 	p4 -c avacomsrc sync //ava.dev/Client/UnrealEngine3/Binaries/...
rem 	p4 -c avacomsrc sync -f //ava.dev/Client/UnrealEngine3/Binaries/*.*
rem 	p4 -c avacomsrc sync //ava.dev/Client/UnrealEngine3/avaGame/...%1
rem 	p4 -c avacomsrc sync //ava.dev/Client/UnrealEngine3/Engine/...%1

cd c:\ue3\UnrealEngine3\binaries
avagame wranglecontent -nosaveunreferenced -cookwindows -unattended LOG=Wrangle.log
rd/s/q c:\deploy\avaGame\Content
rd/s/q c:\deploy\Engine\Content
move c:\ue3\UnrealEngine3\avaGame\CutdownPackages\avaGame\Content c:\deploy\avaGame\Content
move c:\ue3\UnrealEngine3\avaGame\CutdownPackages\Engine\Content c:\deploy\Engine\Content
copy c:\ue3\UnrealEngine3\avaGame\Content\GameRefSC*.upk c:\deploy\avaGame\Content
copy c:\ue3\UnrealEngine3\avaGame\Content\GameLocalSC*.upk c:\deploy\avaGame\Content
cd c:\deploy\Binaries


rem 	cd c:\src\UnrealEngine3\binaries
rem 	avagame wranglecontent -nosaveunreferenced -cookwindows -unattended LOG=Wrangle.log
rem 	rd/s/q c:\deploy\avaGame\Content
rem 	rd/s/q c:\deploy\Engine\Content
rem 	move c:\src\UnrealEngine3\avaGame\CutdownPackages\avaGame\Content c:\deploy\avaGame\Content
rem 	move c:\src\UnrealEngine3\avaGame\CutdownPackages\Engine\Content c:\deploy\Engine\Content
rem 	copy c:\src\UnrealEngine3\avaGame\Content\GameRefSC*.upk c:\deploy\avaGame\Content
rem 	copy c:\src\UnrealEngine3\avaGame\Content\GameLocalSC*.upk c:\deploy\avaGame\Content
rem 	cd c:\deploy\Binaries


:skip_wrangle
del /f c:\deploy\AVA*
del /f c:\deploy\Binaries\*.dmp
del /f c:\deploy\Binaries\AVA.exe
del /f c:\deploy\avaGame\*.bin

del /f c:\deploy\binaries\avakrtest.ini
rem del /f c:\deploy\binaries\*.manifest

rem	ren c:\deploy\Binaries\LTCG-avaGame.exe AVA.exe

copy c:\ue3\unrealengine3\Binaries\LTCG-avaGame.exe c:\deploy\binaries\AVA.exe

del /f c:\deploy\binaries\LTCG-avaGame.exe
del /f c:\deploy\binaries\LTCG-avaGame.ini
del /f c:\deploy\binaries\avaGame.ini

rem copy c:\ue3\unrealengine3\development\avabuild\*.manifest c:\deploy\Binaries
echo > c:\deploy\AVA%1

goto skip_history
rd/s/q \\avabuild\d$\avahistory\3
ren \\avabuild\d$\avahistory\2 3
ren \\avabuild\d$\avahistory\1 2
md \\avabuild\d$\avahistory\1

xcopy /s /d /k /r /y \\avabuild\d$\avadeploy \\avabuild\d$\avahistory\1
 
:skip_history
\\avabuild\ava\robocopy.exe /W:1 /MIR c:\deploy \\avabuild\d$\avadeploy *.* /XF *.log 

