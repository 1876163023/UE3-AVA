//========= Copyright1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VRADDLL_H
#define VRADDLL_H
#ifdef _WIN32
#pragma once
#endif


#include "ivraddll.h"


class CVRadDLL : public IVRadDLL
{
// IVRadDLL overrides.
public:
	virtual int			main( int argc, char **argv );
	virtual bool		Init( char const *pFilename );
	virtual void		Release();
	virtual void		GetBSPInfo( CBSPInfo *pInfo );
	virtual bool		DoIncrementalLight( char const *pVMFFile );
	virtual bool		Serialize();
	virtual float		GetPercentComplete();
	virtual void		Interrupt();
};

//!{ 2006-03-20	Çã Ã¢ ¹Î
class CVRadDLL2 : public IVRadDLL2
{
public:
	// IVRadDLL overrides.
public:
	virtual int			main( int argc, char **argv );
	virtual bool		Init( char const *pFilename );
	virtual void		Release();
	virtual void		GetBSPInfo( CBSPInfo *pInfo );
	virtual bool		DoIncrementalLight( char const *pVMFFile );
	virtual bool		Serialize();
	virtual float		GetPercentComplete();
	virtual void		Interrupt();

	virtual int			FaceLimit();
	virtual int			PlaneLimit();
	virtual int			VertexLimit();
	virtual int			TextureInfoLimit();
	virtual int			TextureDataLimit();
	virtual int			LightLimit();
	virtual int			LightmapSizeLimit();
	virtual int			EdgeLimit();
	virtual int			SurfaceEdgeLimit();
};


#endif // VRADDLL_H
