use threads; 

$addrProgrammers = 'avaprogram@redduck.com';
$addrFrom = 'ava_autobuild';
$addrFail = 'avaprogram@redduck.com';
$addrProgress = 'avaprogram@redduck.com';
$mailserver = 'tomi.redduck.com';
$blat = '_blat.exe';

$justIni = 0;
$doAll = 0;
$builtFromChangelistNum = 0;
$clientspec = "avacomfile";
$filelist = "";
$shaders = 0;
$shader_sm3 = 0;
$shader_sm2 = 0;
$shader_poorsm2 = 0;
$publish = 0;
$uploadSymbols = 0;
$doFullScriptBuild = 0;
$full = 0;

$root = "c:\\ue3";
$ErrorOccurred = 0;
$builtFromChangelistNum = 0;
$doSerial = 0;

$devroot = "$root\\unrealengine3\\development";

$unObjVercpp = "$devroot\\src\\core\\src\\unObjVer.cpp";

$logroot = "\\\\avabuild\\build";

$doMail = 1;
$failSubject = "AVA BUILD FAILED";

$compileSwitches = "/M4";

sub announceProgress
{
	local ($progress) = $_[0];
	
	if ($doMail == 1)
	{
		$attempts = 0;
		$success = 0;
		while ($success == 0 && $attempts < 3)
		{				
			open( SMTP, "|$blat - -t $addrProgress -f $addrFrom -s \"Autobuild progress notification \@$builtFromChangelistNum : $progress\" -server $mailserver > output.txt" );
			print SMTP "AVA build STATUS : $progress..";
			close( SMTP );

			# Make sure we were able to send the mail.
			if (open(OUTPUT,"output.txt"))
			{
				$errors = 0;
				foreach $line (<OUTPUT>)
				{
					if ($line =~ m/Error:/i)
					{
						chomp($line);
						$errors++;
					}
				}
				$success = $errors == 0 ? 1 : 0;
				close(OUTPUT);
			}
			else
			{
				$success = 1;
			}

			# If we failed, delay a few seconds before attempting again.
			if ($success == 0)
			{
				print "Unable to send mail, retrying in 5 seconds...\n";
				sleep(5);
			}
			$attempts++;
		}
	}
}

sub announceFailure
{
	local ($localpart) = $_[0];
	local ($localfile) = $_[1];

	if ( $doMail == 1 )
	{
		$attempts = 0;
		$success = 0;
		while ($success == 0 && $attempts < 10)
		{
			system( "$blat $localfile -html -t $addrFail -f $addrFrom -s \"$failSubject -- $localpart failed.\" -server $mailserver > output.txt" );			

			# Make sure we were able to send the mail.
			if (open(OUTPUT,"output.txt"))
			{
				$errors = 0;
				foreach $line (<OUTPUT>)
				{
					if ($line =~ m/Error:/i)
					{
						chomp($line);
						$errors++;
					}
				}
				$success = $errors == 0 ? 1 : 0;
				close(OUTPUT);
			}
			else
			{
				$success = 1;
			}

			# If we failed, delay a few seconds before attempting again.
			if ($success == 0)
			{
				print "Unable to send mail, retrying in 15 seconds...\n";
				sleep(15);
			}
			$attempts++;
		}
	}
	print "AVA build failure: $localpart failed.\n\nSee $localfile for the build log.\n";
}

sub mailMessage()
{
	my $subject = $_[0];
	local $message = "test";	
	
	open( SMTP, "|\"$blat\" - -t $addrProgrammers -f $addrFrom -s \"$subject\" -server $mailserver" );
	print SMTP "$message\n\n";
	close( SMTP );
}

sub mailFile()
{
	my ($subject,$file) = @_;
	
	system( "\"$blat\" $file -t $addrProgrammers -f $addrFrom -s \"$subject\" -server $mailserver" );	
}

###############################################################################
# incEngineVer
# - Increments the native AVA_VERSION define appearing in $unObjVercpp.
###############################################################################

sub incEngineVer()
{		
    # Grab a copy of the current contents of $unObjVercpp,
	open( VERFILE, "$unObjVercpp" );
	@vertext = <VERFILE>;
	close( VERFILE );
	
	print "\n\n";
		
    # Write $unObjVercpp back out, but with AVA_VERSION bumped up by one.
	open( VERFILE, ">$unObjVercpp" );
	foreach $verline ( @vertext )
	{
		$verline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
		if( $verline =~ m/^#define\sAVA_VERSION\s([0-9]+)$/ )
		{
			$v = $1+1;
			$verline = "#define AVA_VERSION	$v";
			print "버젼이 올라갔습니다 ($v)\n";
		}
		print VERFILE "$verline\n";
	}

	close( VERFILE );


	open( VERFILE, "$unObjVercpp" );
	@vertext = <VERFILE>;
	close( VERFILE );

    # Write $unObjVercpp back out, but with AVA_BUILT_FROM_CHANGELIST set to the changelist that was used to build
	open( VERFILE, ">$unObjVercpp" );
	foreach $verline ( @vertext )
	{
		$verline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
		if( $verline =~ m/^#define\sAVA_BUILT_FROM_CHANGELIST\s([0-9]+)$/ )
		{
			$v = $builtFromChangelistNum;
			$verline = "#define AVA_BUILT_FROM_CHANGELIST $v";
			
			print "Changelist를 기록했습니다 ($v)\n";
		}
		print VERFILE "$verline\n";
	}

	close( VERFILE );	
}

sub setVersion
{
	($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime();
        $versionTimeStamp = sprintf( "%04d-%02d-%02d_%02d.%02d", $year + 1900, $mon + 1, $mday, $hour, $min );
	$version = sprintf( "AVABuild\@%d[$versionTimeStamp]", $builtFromChangelistNum );

	$logfile = "$root\\BuildLogs\\$version.log";
}

sub AppendErrorLog
{
	my ($file) = @_;
	
	if (open( FILE2, "type \"$file\" |"))
	{
		if ($ErrorOccurred == 0)
		{
			open( LOG, "> $logfile" );
			$ErrorOccurred = 1;
		}
		
		print LOG (<FILE2>);
		close( FILE2 );
	}		
}

sub CheckForBuildErrors
{	
	my ($config) = @_;
	
	print "빌드 실패를 검사중입니다 $config\n";
	
	open( FILES, "dir $root\\unrealengine3\\development\\Intermediate\\$config\\BuildLog.htm /s /b |" );

	@files = <FILES>;
	foreach $file (@files)
	{		
		if (open( FILE, "type \"$file\" |"))
		{
			while (<FILE>)
			{
				chomp;			
				
				if ( /.*([0-9]+)\ failed/ )
				{
					if( $1 != 0 )
					{	
						AppendErrorLog( $file );
						
						print "parseNativeLog -- found fails in $file\n";
						break;
					}
				}

				if ( /.*([0-9]+)\ error\(s\)/ )
				{				
					if ( $1 != 0 )
					{			
						AppendErrorLog( $file );
						
						print "parseNativeLog -- found errors in $file\n";
						break;
					}
					else
					{					
						break;
					}
				}			
			}
			close( FILE );
		}
	}
	close( FILES );
	
	if ($ErrorOccurred)
	{
		announceFailure( "Compiling native code", $logfile );
	}
	
	return 1;
}

sub revertAll
{
	# Revert and unlock if we had checked out files.
	system( "p4 -c \"$clientspec\" revert -c default \/\/\.\.\. 2>&1" );	
}

###############################################################################
# checkoutAll
# - Opens the files in @checkoutFiles for editing, and lock them so that other
#   users can't submit until the build is finished.
# - Returns 1 on success or 0 on fail.
###############################################################################

sub checkoutAll
{
	foreach $file (@checkoutFiles)
	{			
		#open( EDIT, "p4 -c \"$clientspec\" edit \"$cspecpath\\$file\" 2>&1 |");
		system( "p4 -c \"$clientspec\" sync \"$file\"");
		open( EDIT, "p4 -c \"$clientspec\" edit \"$file\" 2>&1 |");
		@edit = <EDIT>;
		close( EDIT );

		# If the file was successfully opened for edit, p4 output will be
		# //depot/UnrealEngine3/fileAndPath - opened for edit
		# If the file is already opened for edit by this client, p4 output will be 
		# //depot/UnrealEngine3/fileAndPath - currently opened for edit
		#
		# In both cases we succeed; so, match " opened for edit".  Previously, this check
		# matched "- opened for edit", which could potentially cause a build script
		# to fail if it was run after a previously failed build attempt.

		if( ! ($edit[0] =~ m/\ opened\ for\ edit$/) )
		{
			announceFailure("Perforce checkout","<no log file>");
			print "UE3 build failure: Checkout failed.\n\n$edit[0]";
			return 0;
		}
		else
		{
			# Open for edit was successful; add the file to the list of opened files.
			foreach $line (@edit)
			{
				if ( $line =~ /(.+)\#/ )
				{
					# Add file to the checked out list.
					print "checked out file:$1\n";  #TODO: comment this out again.
					push @checkedoutfiles, $1;
				}
			}
		}
	}

	# Try to lock all the files opened for edit.
	
	# !!! It's a bad idea to fail if locking doesn't succeed.  This is p4 considers a
	# lock attempt to have failed if the client already has a lock, which could
	# conceivably happen if the build script crashes in a bizarre way. !!!
	local $assertLockingSucceeds = 0;   # If 0, don't bother making sure that all files were locked.

	if ( $assertLockingSucceeds == 0 )
	{
		system( "p4 -c $clientspec lock 2>&1" );
	}
	else
	{
		open( LOCKSTATUS, "p4 -c $clientspec lock 2>&1 |" );
		local @output = <LOCKSTATUS>;
		close( LOCKSTATUS );

		# Count up the number of files that we tried to lock but couldn't.

		# If the file locks successfully, p4 output will be:
		# //depot/UnrealEngine3/fileAndPath - locking
		# If the file is already locked, p4 output will be:
		# //depot/UnrealEngine3/fileAndPath - already locked

		$numFilesAlreadyLocked = 0;

		foreach $line (@output)
		{
			if ( ! ($line =~ m/\-\ locking/) )
			{
				$numFilesAlreadyLocked++;
			}
		}
	    
		if ( $numFilesAlreadyLocked != 0 )
		{
			announceFailure( "Lock already exists on $numFilesAlreadyLocked file(s) in checkout list; Perforce checkout", "<no log file>" );
			print "UE3 build failure: Checkout failed; $numFilesAlreadyLocked file(s) already locked\n\n";
			return 0;
		}
    }

	return 1;
}

###############################################################################
# submitAll
# - Unlocks all files listed in @checkedoutfiles and submits them to the depot.
# - Returns 1 on success or 0 on fail.
###############################################################################
# On successful submit, p4 output is:
# Submitting change 75545.
# Locking 1 files ...
# add //depot/UnrealEngine3/Development/Build/dummy2.txt#1
# Change 75545 submitted.
#
# Another successful submit:
# Submitting change 75605.
# Locking 2 files ...
# edit //depot/UnrealEngine3/Development/Build/dummy.txt#5
# delete //depot/UnrealEngine3/WarGame/Content/wargameContent.txt#2
# Change 75605 renamed change 75608 and submitted.
#
# On unsuccessful submit, p4 output is:
# Submitting change 75544.
# //depot/UnrealEngine3/Development/Build/dummy.txt - already locked
# File(s) couldn't be locked.
# Submit failed -- fix problems above then use 'p4 submit -c 75544'.

sub submitAll
{
	my ($reason) = @_;
	# Unlock the files for check in.  Even though this unlock looks redundant,
	# because your locks are released automatically on submission, if submit fails
	# then the locks won't be released, so we must manually unlock.
        # msew: this is bad as during the unlock I could edit and submit a file and cause a build failure
        #       if the build fails then we need to a) revert
	#system( "p4 -c $clientspec unlock 2>&1" );

	# Revert unchanged files.
	# No need to revert unchanged files.  Submitting unchanged files does no harm. 
        # msew: we want to revert unchanged files as binary files take up their compressed size
        #       we also want to revert unchanged files as it makes looking at the history of changes in the file impossible
        #     But we CAN NOT do this.  the @checkedoutfiles list contains the set of files we are
        #     expecting to check in.  This stops us from reverting unchanged files
	#system( "p4 -c $clientspec revert -a 2>&1" );

	open( SUBMITCMD, "|p4 -c \"$clientspec\" submit -i > output.txt 2>&1 " );

	print SUBMITCMD "Change:\tnew\n";
	print SUBMITCMD "Client:\t$clientspec\n";
	print SUBMITCMD "Status:\tnew\n";		
	print SUBMITCMD "Description:\t$version\n $reason";
	
	print SUBMITCMD "\n";
		
	print SUBMITCMD "Files:\n";

	foreach $file (@checkedoutfiles)
	{
		print SUBMITCMD "\t$file#edit\n";
	}
	
	close( SUBMITCMD );

	# Was the submit successful?
	local $foundSuccess = 0;
	local $accumSubmitLog = "";
	if (open(OUTPUT,"output.txt") )
	{
		foreach $line (<OUTPUT>)
		{
			$accumSubmitLog .= $line;
			if ( $line =~ m/([0-9]+) submitted/i )
			{
			    $submittedChangeListNum = $1;
				$foundSuccess = 1;
				last;
			}
			elsif ( $line =~ m/([0-9]+) and submitted/i )		# In the case of Perforce renaming.
			{
			    $submittedChangeListNum = $1;
				$foundSuccess = 1;
				last;
			}
		}
		close( OUTPUT );
	}
	else
	{
		$foundSuccess = 1;
	}

	if ( $foundSuccess == 0 )
	{
		$failLogName = "submitFail_".$version.".log";
		open( FAILLOG, "> $failLogName" );
		print FAILLOG $accumSubmitLog;
		print FAILLOG "Probable cause: game/script compile error prevented the generation of a file that was flagged for submission.\n";
		print FAILLOG "See compile logs to verify.\n";
		close( FAILLOG );
		system("copy $failLogName $failedLogDir /y" );
		announceFailure( "Perforce submit (untrapped compile fail?)", $failedLogDirLink.$failLogName );
	}

	return $foundSuccess;
}

###############################################################################
# announceFailure
#
###############################################################################

sub announceFailure
{
	local ($localpart) = $_[0];
	local ($localfile) = $_[1];

	if ( $doMail == 1 )
	{
		$attempts = 0;
		$success = 0;
		while ($success == 0 && $attempts < 10)
		{
			open( SMTP, "|\"$blat\" - -t $addrFail -f $addrFrom -s \"$failSubject -- $localpart failed.\" -server $mailserver > output.txt" );
			print SMTP "UE3 build failure: $localpart failed.\n\nSee $localfile for the build log.";
			close( SMTP );

			# Make sure we were able to send the mail.
			if (open(OUTPUT,"output.txt"))
			{
				$errors = 0;
				foreach $line (<OUTPUT>)
				{
					if ($line =~ m/Error:/i)
					{
						chomp($line);
						$errors++;
					}
				}
				$success = $errors == 0 ? 1 : 0;
				close(OUTPUT);
			}
			else
			{
				$success = 1;
			}

			# If we failed, delay a few seconds before attempting again.
			if ($success == 0)
			{
				print "Unable to send mail, retrying in 15 seconds...\n";
				sleep(15);
			}
			$attempts++;
		}
	}
	print "UE3 build failure: $localpart failed.\n\nSee $localfile for the build log.\n";
}

sub parseCommandLineArgs
{
	foreach $argnum (0 .. $#ARGV )
	{
		$curArg = $ARGV[$argnum];
		
		print "argument - $curArg\n";
		
		if ($curArg eq "justini")
		{
			$justIni = 1;
			
			print "Encrypt만 수행합니다\n";
		}
		elsif ($curArg eq "all")
		{
			$doAll = 1;
			
			print "LTCG도 컴파일합니다\n";
		}		
		elsif ($curArg eq "shaders")
		{
			$shaders = 1;			
			$shader_sm3 = 1;
			$shader_sm2 = 1;
			$shader_poorsm2 = 1;
			
			print "Shader를 컴파일합니다\n";
		}		
		elsif ($curArg eq "shader-sm3")
		{
			$shaders = 1;			
			$shader_sm3 = 1;			
			
			print "Shader를 컴파일합니다\n";
		}		
		elsif ($curArg eq "shader-sm2")
		{
			$shaders = 1;			
			$shader_sm2 = 1;			
			
			print "Shader를 컴파일합니다\n";
		}		
		elsif ($curArg eq "shader-poorsm2")
		{
			$shaders = 1;			
			$shader_poorsm2 = 1;			
			
			print "Shader를 컴파일합니다\n";
		}				
		elsif ($curArg eq "sym")
		{
			$uploadSymbols  = 1;
			
			print "Symbol을 upload합니다\n";
		}
		elsif ($curArg eq "full")
		{
			$doFullScriptBuild = 1;
			
			print "script를 완전히 빌드합니다\n";
		}
		elsif ($curArg eq "serial")
		{
			$doSerial = 1;
			
			print "병렬 처리 하지 않습니다\n";
		}
		elsif ($curArg eq "publish")
		{
			$publish = 1;
			$doAll = 1;
			$uploadSymbols = 1;
			
			print "Deploy requested\n";
		}
		elsif ($curArg eq "clean")
		{
			$full = 1;
			
			print "Full rebuild requested\n";
		}
	}
}

sub grabChangelist
{
	open( CHANGES, "p4 changes -m 1 |" );
	foreach $line (<CHANGES>)
	{
		# Is this a build version change?
		if ( $line =~ /^Change ([0-9]+) on ([0-9]{4})\/([0-9]{2})\/([0-9]{2}).+by/ )
		{
			$builtFromChangelistNum = $1;
		}
	}
	close( CHANGES );
}

sub sync_explicit
{	
	@explicit_sync = (25165);	
	$builtFromChangelistNum = 25106;	
	
	system( "p4 -c $clientspec sync //ava.dev/...\@$builtFromChangelistNum" );	
	
	foreach $explicit_sync_changelist (@explicit_sync)
	{
		system( "p4 -c $clientspec sync \@=$explicit_sync_changelist" );	
	}	
}


sub sync
{	
	#sync_explicit;
	system( "p4 -c $clientspec sync //ava.dev/...\@$builtFromChangelistNum" );		
}

sub compileRelease
{
	if ($full)
	{
		system( "vcbuild /clean $compileSwitches ava.sln \"releaseNet|Win32\"" );	
	}
	return system( "vcbuild $compileSwitches ava.sln \"releaseNet|Win32\"" );	
}

sub compileLTCG
{
	if ($full)
	{
		system( "vcbuild /clean $compileSwitches avaLTCG.sln \"releaseLTCG|Win32\"" );	
	}
	return system( "vcbuild $compileSwitches avaLTCG.sln \"releaseLTCG|Win32\"" );
}

sub compileNative
{	
	incEngineVer();
	
	if ($doAll == 0)
	{
		compileRelease && CheckForBuildErrors("Release") && $ErrorOccurred && return 0;		
	}
	else
	{
		local $a, $b;
		
		if ($doSerial)
		{
			$a = complieRelease;
			$b = compileLTCG;
		}
		else
		{
			$thr1 = threads->new(\&compileRelease);
			$thr2 = threads->new(\&compileLTCG);		
		
			$a = $thr1->join; 
			$b = $thr2->join; 
		}
	    
	    (!$a || !$b) && CheckForBuildErrors("Release") && CheckForBuildErrors("ReleaseLTCG") && $ErrorOccurred && return 0;	    	    
	    
	    print "\nEncrypting EXE with NPGE...\n";
	    system( "echo > npge.txt" );
	    system("NPGEClient.exe /ava:ava!@#:5:\"..\\..\\Binaries\\LTCG-avaGame.exe\";NPGE") == 0 or return 0;
	    system( "del npge.txt" );
	}	
	
	print "\n빌드 성공!\n";
	
	system("touch $root\\unrealengine3\\binaries\\avaGame.exe");
	
	if ($doAll == 1)
	{
		system("touch $root\\unrealengine3\\binaries\\LTCG-avaGame.exe");
	}
	
	return 1;
}

sub compileScripts
{
	local $option = "";
	
	print "\n스크립트 빌드 시작!\n";
	
	$option = "$option -full -cook";
	
	system( "del ..\\..\\avaGame\\Script\\ava*.u" );
	system( "..\\..\\binaries\\avagame.com make -hidesplash -unattended -nopause $option" ) && return 0;
	
	if ($doAll == 0)
	{
		return 1;
	}


	system( "rd/s/q ..\\..\\avaGame\\Script__" );
	system( "ren ..\\..\\avaGame\\Script Script__" );
	system( "del ..\\..\\avaGame\\ScriptFinalRelease\\ava*.u" );
	system( "..\\..\\binaries\\avagame.com make -hidesplash -unattended -nopause -final_release $option" )  && return 0;	
	system( "ren ..\\..\\avaGame\\Script__ Script" );
	
	print "\n스크립트 빌드 성공!\n";	
	
	return 1;
}

sub encryptIni
{
	print "\n암호화!\n";
	    
	system( "..\\..\\binaries\\avagame.com encryptini -hidesplash -unattended -nopause" );
	# && return 0;	
	
	return 1;
}

sub prepare
{
	@checkoutFiles = ();

	if ($shaders)
	{
		if ($shader_sm3)
		{
			push(@checkoutFiles,'//ava.dev/Client/UnrealEngine3/avaGame/Content/RefShaderCache-PC-D3D.upk');
			push(@checkoutFiles,'//ava.dev/Client/UnrealEngine3/avaGame/Content/GameRefSC-PC-D3D.upk');
		}
		
		if ($shader_sm2)
		{
			push(@checkoutFiles,'//ava.dev/Client/UnrealEngine3/avaGame/Content/RefShaderCache-PC-D3D-SM2.upk');
			push(@checkoutFiles,'//ava.dev/Client/UnrealEngine3/avaGame/Content/GameRefSC-PC-D3D-SM2.upk');
		}
		
		if ($shader_poorsm2)
		{
			push(@checkoutFiles,'//ava.dev/Client/UnrealEngine3/avaGame/Content/RefShaderCache-PC-D3D-SM2P.upk');
			push(@checkoutFiles,'//ava.dev/Client/UnrealEngine3/avaGame/Content/GameRefSC-PC-D3D-SM2P.upk');
		}		
		
		checkoutAll();		
		
		return 1;
	}
	else
	{
		if ($justIni)
		{
			$filelist = "deployFileList_Ini.txt";
		}
		elsif ($doAll)
		{
			$filelist = "deployFileList_All.txt";
		}
		else
		{
			$filelist = "deployFileList.txt"
		}
	}	
	
	if ( open( CHECKOUTFILELIST, $filelist ) )
	{
		# Grab the list of include paths.
		chomp( @checkoutFiles = <CHECKOUTFILELIST> );     # Strip trailing newlines.
		close( CHECKOUTFILELIST );	

		checkoutAll();		
		
		close( CHECKOUTFILELIST );
	}
	else
	{
		return 0;
	}	
	
	
	return 1;
}

sub labelAll
{
	my ($labelName) = @_;
	
	print "\n\n##### Labeling $labelName\n";
	
	open( LABELCMD, "|p4 -c $clientspec label -i" );

	print LABELCMD "Label: $labelName\n";
	print LABELCMD "Owner: deif\n";
	print LABELCMD "Description: Auto created\n";		
	print LABELCMD "Options: unlocked\n";
	print LABELCMD "View:\n";
	print LABELCMD "\t//ava.dev/Client/UnrealEngine3/Binaries/...\n";
	print LABELCMD "\t//ava.dev/Client/UnrealEngine3/avaGame/...\n";				
	print LABELCMD "\t//ava.dev/Client/UnrealEngine3/Engine/...\n";				

	close( LABELCMD );

	system( "p4 -c $clientspec labelsync -l $labelName" );
};

sub stripOutDevMaps
{
	open( INIFILE, "$root\\unrealengine3\\avaGame\\Config\\DefaultNet.ini" );	

	@INI = <INIFILE>;

	close (INIFILE);

	open( INIFILE, "> $root\\unrealengine3\\avaGame\\Config\\DefaultNet.ini" );

	foreach $line (@INI)
	{
		if ( $line =~ m/^MapList=/ && $line =~ m/\(Dev\)/)
		{
			print $line;
		}
		else
		{
			print INIFILE $line;
		}
	}

	close( INIFILE );
};

sub doFinalDeploy_EXPERIMENTAL
{	
	($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
	
	$INTERNAL_LABEL_FOR_PRE_PUBLISH = "INTERNAL_LABEL_FOR_PRE_PUBLISH";
	labelAll( $INTERNAL_LABEL_FOR_PRE_PUBLISH );

	@checkoutFiles = ();
	
	open( CHECKOUTFILELIST, "deployFileList_Ini.txt" ) || return 0;
	
	# Grab the list of include paths.
	chomp( @checkoutFiles = <CHECKOUTFILELIST> );     # Strip trailing newlines.
	close( CHECKOUTFILELIST );	
	
	push(@checkoutFiles, "//ava.dev/Client/UnrealEngine3/avaGame/Config/DefaultNet.ini");

	checkoutAll();				
	
	stripOutDevMaps();
	
	encryptIni();
	
	submitAll( "strip out dev resources for publish" );	
	
	$published_label = sprintf "PUBLISH_%04d_%02d_%02d_%02d_%02d_%02d", (1900+$year), $mon, $mday, $hour, $min, $sec;
	
	labelAll( $published_label );
	
	system("finalDeploy.bat \@$published_label" );
	
	foreach $file (@checkOutFiles)
	{	
		system( "p4 -c $clientspec sync $file\@$INTERNAL_LABEL_FOR_PRE_PUBLISH" );
	}	
	
	checkoutAll();
	
	foreach $file (@checkOutFiles)
	{	
		system( "p4 -c $clientspec sync $file" );
		system( "p4 -c $clientspec resolve -ay $file" );
	}		
	
	submitAll( "reverted publish-related changes" );
	
	return 1;
}

sub doFinalDeploy
{	
	system("finalDeploy.bat \@LATEST_ENGINE" );
	
	return 1;
}

sub deploy
{
	print "\n배포!\n";
	
	submitAll( "built from changelist:  $builtFromChangelistNum" );
	
	if (!$shaders)
	{
		labelAll( "LATEST_ENGINE" );
	}	
	
	return 1;
}

sub compileShaderForPlatform
{
	my ($id, $name) = @_;
		
	if ($full)
	{
		system("del /f ..\\..\\avagame\\content\\gamerefsc-$name.upk");
		system("del /f ..\\..\\avagame\\content\\refshadercache-$name.upk");
		
		system("del /f ..\\..\\avagame\\content\\gamelocalsc-$name.upk");
		system("del /f ..\\..\\avagame\\content\\localshadercache-$name.upk");
	}
			
	system("del /f ..\\..\\avagame\\content\\gamelocalsc-$name.upk");
	system("del /f ..\\..\\avagame\\content\\localshadercache-$name.upk");
	
	system("copy /y ..\\..\\avagame\\content\\refshadercache-$name.upk ..\\..\\avagame\\content\\localshadercache-$name.upk");
	system("copy /y ..\\..\\avagame\\content\\gamerefsc-$name.upk ..\\..\\avagame\\content\\gamelocalsc-$name.upk");
	
	system("del /f ..\\..\\avagame\\content\\refshadercache-$name.upk");
	system("del /f ..\\..\\avagame\\content\\gamerefsc-$name.upk");
	
	system("..\\..\\binaries\\avaGame.com precompileshaders platform=$id -hidesplash -unattended -nopause -skipmaps -ALLOW_PARALLEL_PRECOMPILESHADERS LOG=Launch$id.log");
	system("..\\..\\binaries\\avaGame.com precompileshaders platform=$id -hidesplash -game -unattended -nopause -skipmaps -ALLOW_PARALLEL_PRECOMPILESHADERS LOG=Launch$id.log");
		
	system("copy /y ..\\..\\avagame\\content\\localshadercache-$name.upk ..\\..\\avagame\\content\\refshadercache-$name.upk");
	system("copy /y ..\\..\\avagame\\content\\gamelocalsc-$name.upk ..\\..\\avagame\\content\\gamerefsc-$name.upk");
	
	return 1;
}

sub compileShaderForPCD3D
{
	return compileShaderForPlatform( "pc", "PC-D3D" );
}

sub compileShaderForPCD3D_SM2
{
	return compileShaderForPlatform( "pc_sm2", "PC-D3D-SM2" );
}

sub compileShaderForPCD3D_SM2_POOR
{
	return compileShaderForPlatform( "pc_sm2_poor", "PC-D3D-SM2P" );
}

sub clearAllStat
{
	system( "del /f $logroot\\.*" );
}

sub compileShaders
{
	print "\n\nCompile shaders!!!!!!!!!!!!!!!\n";

	system("attrib -r ..\\..\\avagame\\content\\*.upk");
	
	if ($doSerial)
	{
		if ($shader_sm3)
		{
			(!compileShaderForPCD3D) && return 0;
		}
		
		if ($shader_sm2)
		{
			(!compileShaderForPCD3D_SM2) && return 0;
		}
		
		if ($shader_poorsm2)
		{
			(!compileShaderForPCD3D_SM2_POOR) && return 0;
		}
		
		return 1;
	}
	
	local $a, $b, $c, $thr1, $thr2, $thr3;
	
	$a = 1;
	$b = 1;
	$c = 1;
		
	if ($shader_sm3)
	{
		$thr1 = threads->new(\&compileShaderForPCD3D);
	}
	
	if ($shader_sm2)
	{		
		$thr2 = threads->new(\&compileShaderForPCD3D_SM2);		
	}
	
	if ($shader_poorsm2)
	{
		$thr3 = threads->new(\&compileShaderForPCD3D_SM2_POOR);
	}

	if ($shader_sm3)
	{
		$a = $thr1->join; 
	}
	
	if ($shader_sm2)
	{
		$b = $thr2->join; 
	}
	
	if ($shader_poorsm2)
	{
		$c = $thr3->join;
	}
    
	return $a && $b && $c;
}

sub doBuild
{
#	return 1;
	print "\n\nDO BUILD!\n========\n";
	
	if ($shaders)
	{
		return compileShaders;
	}
	elsif ($justIni)
	{
		return encryptIni;
	}
	else
	{
		return compileNative && compileScripts && encryptIni;
	}
}

clearAllStat;

revertAll;

parseCommandLineArgs;

grabChangelist;

setVersion;

#$builtFromChangelistNum = 18909;

print "처리 중인 changelist : $builtFromChangelistNum $version\n";

announceProgress( "BEGIN" );

sync;

#system( "p4 -c $clientspec sync -f //ava.dev/Client/UnrealEngine3/Development/Src/avaGame/Classes/avaGameViewportClient.uc" );

if (!(prepare && doBuild && deploy))
{
	revertAll;
}
else
{
	if ($publish)
	{	
		doFinalDeploy;
	}
	
	if ($uploadSymbols == 1)
	{		
		system("store_symbols_ava $builtFromChangelistNum $version");     
	}
}

if ($ErrorOccurred != 0)
{
	close( LOG );
}

clearAllStat;

announceProgress( "COMPLETED" );