/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_MovieTexture extends SequenceAction;	

enum MovieTextureOp
{
	MTO_Play,	
	MTO_Pause,
	MTO_Stop,
	MTO_Activate,
	MTO_Deactivate,
};

/** player that should take credit for the damage (Controller or Pawn) */
var() TextureMovie  Texture;

var() MovieTextureOp Operation;

event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

event Activated()
{
	Local array<TextureMovie> TextureMovies;
	Local TextureMovie Movie;

	foreach Targets(Movie)
	{
		TextureMovies.AddItem(Movie);
	}
	if( Texture != None )
		TextureMovies.AddItem(Texture);

	foreach TextureMovies(Movie)
	{
		if (Movie != None)
		{
			if (Operation == MTO_Play)
				Movie.Play();
			else if (Operation == MTO_Pause)
				Movie.Pause();
			else if (Operation == MTO_Stop)
				Movie.Stop();
		}
	}
}

defaultproperties
{
	ObjName="Movie Texture"
	ObjCategory="Misc"
	ObjClassVersion=2	

	Operation = MTO_Play
}
