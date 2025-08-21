#include "PrecompiledHeaders.h"
#include "avaGame.h"

IMPLEMENT_CLASS(UavaUtil);
IMPLEMENT_CLASS(UavaCache);

void UavaUtil::UpdateTextures(const TArray<class UTexture*>& Textures)
{
	int NumMax = Textures.Num() > 0 ? Textures.Num() - 1 : 0;

	for ( INT i = 0; i < Textures.Num(); ++i )
	{
		if ( Textures(i) == NULL )
			continue ;

		if (Textures(i)->LODGroup != TEXTUREGROUP_UI)
		{
			Textures(i)->UpdateResource();

			debugf(TEXT("UpdateTexture(%s) [%d/%d]"), *Textures(i)->GetName(), i, NumMax);
		}
	}
}

float UavaUtil::GetSeconds()
{
	return (float)appSeconds();
}

UavaCache *GAvaCache = NULL;

UavaCache* UavaCache::GetInstance()
{
	if (!GAvaCache)
	{
		GAvaCache = ConstructObject<UavaCache>(UavaCache::StaticClass(), INVALID_OBJECT, NAME_None, RF_Keep);
		check(GAvaCache);

		GAvaCache->AddToRoot();
	}

	return GAvaCache;
}