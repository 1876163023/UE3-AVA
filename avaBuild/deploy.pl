#자동으로 찾게 해놨습니다. :)
$clientspec = "avacomfile";

$builtFromChangelistNum = $ARGV[0];
if (!$builtFromChangelistNum)
{
	$builtFromChangelistNum = "{unknown}";
}

if ( open( CHECKOUTFILELIST, $ARGV[1] ) )
{
	# Grab the list of include paths.
	chomp( @checkoutFiles = <CHECKOUTFILELIST> );     # Strip trailing newlines.
	close( CHECKOUTFILELIST );	

	checkoutAll();
	submitAll();
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
	print SUBMITCMD "Description:\t$version\n built from changelist:  $builtFromChangelistNum";
	
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