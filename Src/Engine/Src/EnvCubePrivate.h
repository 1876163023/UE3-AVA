#ifndef __ENVCUBEPRIVATE_H__
#define __ENVCUBEPRIVATE_H__

/* Render Thread */
void EnvCube_ResetCache();
void EnvCube_PrepareDraw();
void EnvCube_Reserve( const FShaderParameter& Parameter, UINT ElementIndex = 0 );
void EnvCube_Bind( FCommandContextRHI* Context, FPixelShaderRHIParamRef PixelShader, const FPrimitiveSceneInfo* PrimitiveSceneInfo );
void EnvCube_MarkDirty();
void EnvCube_Update();

FORCEINLINE FString EnvCube_GetCubeTexName( AEnvCubeActor* EnvCube )
{
	return FString::Printf( TEXT("%s_Cube"), *EnvCube->GetName() );
}

FORCEINLINE FString EnvCube_GetCubeFaceTexName( AEnvCubeActor* EnvCube, INT FaceIndex )
{
	return FString::Printf( TEXT("%s_Cube%d"), *EnvCube->GetName(), FaceIndex );
}

FORCEINLINE UTextureCube* EnvCube_GetCubeTex( AEnvCubeActor* EnvCube )
{
	return FindObject<UTextureCube>( EnvCube->TexturePackage ? EnvCube->TexturePackage : EnvCube->GetOutermost(), *EnvCube_GetCubeTexName( EnvCube ), TRUE );	
}

FORCEINLINE UTexture2D* EnvCube_GetCubeFaceTex( AEnvCubeActor* EnvCube, INT FaceIndex )
{
	return FindObject<UTexture2D>( EnvCube->TexturePackage ? EnvCube->TexturePackage : EnvCube->GetOutermost(), *EnvCube_GetCubeFaceTexName( EnvCube, FaceIndex ), TRUE );	
}

extern UBOOL GEnvCube_IsRequired;

#endif // __ENVCUBEPRIVATE_H__