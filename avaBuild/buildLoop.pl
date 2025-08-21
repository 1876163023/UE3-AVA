#!/usr/bin/perl.exe

$root = "\\\\avabuild\\build";

$clientspec       = 'avacomfile';
$release = "$root\\release.txt";
$all = "$root\\all.txt";
$clean = "$root\\clean.txt";
$justini = "$root\\ini.txt";
$publish = "$root\\publish.txt";
$shaders = "$root\\shaders.txt";
$shader_sm3 = "$root\\shader-sm3.txt";
$shader_sm2 = "$root\\shader-sm2.txt";
$shader_poorsm2 = "$root\\shader-poorsm2.txt";
$nobuild = "$root\\nobuild.txt";
$buildscript = "build.pl";

$ad_ini = "$root\\INI��ȣȭ��ûȮ��";
$ad_all = "$root\\LTCG������ûȮ��";
$ad_publish = "$root\\������ûȮ��";
$ad_release = "$root\\Release����ûȮ��";
$ad_shaders = "$root\\Shader��ûȮ��";

# Make sure everything is cleaned up from a halted build.
system( "p4 -c \"$clientspec\" revert -c default \/\/\.\.\. 2>&1" );
system( "del $buildflag /f" );

while(1)
{	
	local $opt = "";
	local $shaderopt = "";
	local $doshader = 0;
	local $idle = 0;
	local $job = "";
	
	if( -e $clean )
	{
		$opt = "$opt clean";
		system( "del $clean /f" );
	}
	
	if( -e $shader_sm3 )
	{
		$shaderopt = "$shaderopt shader-sm3";
		$doshader = 1;
		system( "del $shader_sm3 /f" );
	}
	
	if( -e $shader_sm2 )
	{
		$shaderopt = "$shaderopt shader-sm2";
		$doshader = 1;		
		system( "del $shader_sm2 /f" );
	}
	
	if( -e $shader_poorsm2 )
	{
		$shaderopt = "$shaderopt shader-poorsm2";
		$doshader = 1;		
		system( "del $shader_poorsm2 /f" );
	}
	
	if( -e $shaders )
	{
		$shaderopt = "$shaderopt shaders";
		$doshader = 1;
		
		system( "del $shaders /f" );
	}	
	
	if( ( -e $publish ) && ( !( -e $nobuild ) ) )
	{
		$job = "����[PUBLISH]";
		print "Doing job $job\n";
		system( "echo > $ad_publish" );
		system( "$buildscript publish $opt" );
		system( "del $publish /f" );
		system( "del $all /f" );
		system( "del $release /f" );
		system( "del $ad_publish /f" );		
	}
	elsif( ( -e $all ) && ( !( -e $nobuild ) ) )
	{		
		$job = "����[ReleaseNet + ReleaseLTCG]";
		print "Doing job $job\n";
		system( "echo > $ad_all" );
		system( "$buildscript all $opt" );
		system( "del $all /f" );
		system( "del $release /f" );
		system( "del $ad_all /f" );		
	}
	elsif( ( -e $justini ) && ( !( -e $nobuild ) ) )
	{		
		$job = "��ȣȭ[INI]";
		print "Doing job $job\n";
		system( "echo > $ad_ini" );
		system( "$buildscript justini" );		
		system( "del $justini /f" );		
	}
	elsif( ( -e $release ) && ( !( -e $nobuild ) ) )
	{
		$job = "����[ReleaseNet]";
		print "Doing job $job\n";
		system( "echo > $ad_release $opt" );
		system( "$buildscript" );		
		system( "del $release /f" );
		system( "del $ad_release /f" );		
	}
	elsif( ( $doshader ) && ( !( -e $nobuild ) ) )
	{
		$job = "���̴�[$shaderopt]";
		print "Doing job $job\n";
		system( "echo > $ad_shaders" );
		system( "$buildscript $shaderopt $opt serial" );						
		system( "del $ad_shaders /f" );		
	}		
	else
	{
		sleep(10);

		$idle = 1;		
	}
	
	if (!$idle)
	{
		print "====================================================\n$job completed\n";
	}

	if( -e $nobuild )
	{
		print "���� ���� ��û�����Դϴ�.\n";
	}
}