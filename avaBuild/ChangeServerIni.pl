$gameini = "c:\\deploy\\avagame\\config\\defaultgame.ini";
$serverlist = "211.233.47.13:28004";
$udpserverlist = "211.233.47.16:28020";

open( INIFILE, "$gameini" );
@vertext = <INIFILE>;
close( INIFILE );
	
print "\n\n";
		
# Write $gameini back out, but with AVA_VERSION bumped up by one.
open( INIFILE, ">$gameini" );
foreach $iniline ( @vertext )
{
	$iniline =~ s/\n//;   # Remove first newline occurance (hopefully at end of line...)
	if( $iniline =~ m/^ServerList=.*$/ )
	{
		$iniline = "ServerList=$serverlist";
		print "server list replaced\n";
	}
	elsif( $iniline =~ m/^UdpServerList=.*$/)
	{
		$iniline = "UdpServerList=$udpserverlist";
		print "udp server list replaced\n";
	}
	print INIFILE "$iniline\n";
}

close( INIFILE );