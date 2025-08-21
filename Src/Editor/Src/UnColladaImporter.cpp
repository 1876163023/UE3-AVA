/*=============================================================================
	COLLADA importer for Unreal Engine 3.
	Based on top of the Feeling Software's Collada import classes [FCollada].
	Copyright 2006 Epic Games, Inc. All Rights Reserved.

	Class implementation inspired by the code of Richard Stenson, SCEA R&D
==============================================================================*/

#include "EditorPrivate.h"
#include "EngineAnimClasses.h"
#include "UnColladaImporter.h"
#include "UnColladaSceneGraph.h"
#include "UnColladaSkeletalMesh.h"
#include "UnColladaStaticMesh.h"
#include "UnCollada.h"

namespace UnCollada {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CImporter - Main COLLADA CImporter Class
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CImporter::CImporter()
	:	ColladaDocument( NULL )
	,	bParsingSuccessful( FALSE )
{
}

CImporter::~CImporter()
{
//	delete ColladaDocument;
}

/**
 * Returns the importer singleton. It will be created on the first request.
 */
CImporter* CImporter::GetInstance()
{
	static CImporter* ImporterInstance = new CImporter();
	return ImporterInstance;
}

/**
 * Attempt to import the given COLLADA text string.
 * Returns TRUE on success. This is the same value returned by 'IsParsingSuccessful'.
 */
UBOOL CImporter::ImportFromText(const FString& Filename, const TCHAR* FileStart, const TCHAR* FileEnd)
{
	return FALSE;
}

/**
 * Attempt to import a COLLADA document from a given filename
 * Returns TRUE on success. This is the same value returned by 'IsParsingSuccessful'.
 */
UBOOL CImporter::ImportFromFile(const TCHAR* Filename)
{
	return FALSE;
}

/**
 * Retrieves a list of all the meshes/controllers within the last successfully parsed COLLADA document.
 * Do not hold onto the returned strings.
 */
void CImporter::RetrieveEntityList(TArray<const TCHAR*>& EntityNameList, UBOOL bSkeletalMeshes)
{	
}

/**
 * Creates a static mesh with the given name and flags, imported from within the COLLADA document.
 * Returns the UStaticMesh object.
 */
UObject* CImporter::ImportStaticMesh(UObject* InParent, const TCHAR* ColladaName, const FName& Name, EObjectFlags Flags)
{
	return NULL;
}

/**
 * Retrieve an engine material instance, given a COLLADA geometry instance and a set of associated polygons
 */
UMaterialInstance* CImporter::FindMaterialInstance(const FCDGeometryInstance* ColladaInstance, const FCDGeometryPolygons* ColladaPolygons)
{
	return NULL;
}

/**
 * Creates a skeletal mesh with the given name and flags, imported from within the COLLADA document.
 * Returns the USkeletalMesh object.
 */
UObject* CImporter::ImportSkeletalMesh(UObject* InParent, const TArray<FString>& ColladaNames, const FName& Name, EObjectFlags Flags)
{
	return NULL;	
}

/**
 * Add to the animation set, the animations contained within the COLLADA document, for the given skeletal mesh
 */
void CImporter::ImportAnimSet(UAnimSet* AnimSet, USkeletalMesh* Mesh, const TCHAR* AnimSequenceName)
{
}

#define SET_ERROR(ErrorType) { if (Error != NULL) *Error = ErrorType; }

/**
 * Add to the morpher set, the morph target contained within the COLLADA document, for the given skeletal mesh
 */
void CImporter::ImportMorphTarget(USkeletalMesh* Mesh, UMorphTargetSet* MorphSet, TArray<FName> MorphTargetNames, TArray<const TCHAR*> ColladaNames, UBOOL bReplaceExisting, EMorphImportError* Error)
{	
}

} // namespace UnCollada
