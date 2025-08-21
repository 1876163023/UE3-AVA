/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaLinkedReplicationInfo extends ReplicationInfo
	abstract
	native
	nativereplication;

var avaLinkedReplicationInfo NextReplicationInfo;

cpptext
{
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	// Variables the server should send to the client.
	if ( bNetInitial && (Role==ROLE_Authority) )
		NextReplicationInfo;
}

defaultproperties
{
	NetUpdateFrequency=1
}
