/*=============================================================================
	UnObjVer.h: Unreal object version.
	Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Prevents incorrect files from being loaded.

#define PACKAGE_FILE_TAG			0x9E2A83C1
#define PACKAGE_FILE_TAG_SWAPPED	0xC1832A9E

//
//	Package file version log:
//
// 224:
// -	Removing Length, XSize, YSize and ZSize from VJointPos
#define VER_REMOVE_SIZE_VJOINTPOS							224
// 225:
// -	Added BackfaceShadowTexCoord to FVert.
#define VER_BACKFACESHADOWTEXCOORD							225
// 226:
// -	Added ThumbnailDistance to the natively serialized StaticMesh.
#define VER_STATICMESH_THUMBNAIL_DISTANCE					226
// 227:
// -	Converted FPoly::Vertex to a TArray.
#define VER_FPOLYVERTEXARRAY								227
// 228:
// -	Converted FQuantizedLightSample to use FColor instead of BYTE[4]. Needed to byte swap correctly on Xenon
#define	VER_QUANT_LIGHTSAMPLE_BYTE_TO_COLOR					228
// 229:
// -	Low poly collision data for terrain is serialized
#define VER_TERRAIN_COLLISION								229
// 230:
// -	Added texture LOD groups. Version incremented to set default values.
#define VER_TEXTURE_GROUPS									230
// 231:
// -	Converted ULightMap1D::Scale to FLOAT[3] instead of FLinearColor
#define	VER_LIGHTMAP_SCALE_TO_FLOAT_ARRAY					231
//	232:
//	-	Added decal manager to ULevel.
#define VER_ADDED_DECAL_MANAGER								232
//	233:
//	-	Added ParticleModuleRequired to support LOD levels in particle emitters.
//	-	Changed to LOD model for emitters.
#define VER_CHANGE_EMITTER_TO_LODMODEL						233
//	234:
//	-	Changed streaming code around to split texture and sound streaming into separate arrays
#define VER_SPLIT_SOUND_FROM_TEXTURE_STREAMING				234
//	235:
//	-	Serialize terrain patch bounds
#define VER_SERIALIZE_TERRAIN_PATCHBOUNDS					235
//  236:
//	-	Add structures to BSP and terrain for pre-cooked Novodex collision data.
#define VER_PRECOOK_PHYS_BSP_TERRAIN						236
//  237:
//	-	Add structures to BrushComponent for pre-cooked Novodex collision data.
#define VER_PRECOOK_PHYS_BRUSHES							237
//  238:
//	-	Add pre-cooked static mesh data cache to ULevel
#define VER_PRECOOK_PHYS_STATICMESH_CACHE					238
//	239:
//	-	UDecalComponent's RenderData is now serialized.
#define VER_DECAL_RENDERDATA								239
//	240 
//	-	UDecalComponent's RenderData is now serialized as pointers.
#define VER_DECAL_RENDERDATA_POINTER						240
//	241 
//	-	Repairs bad centroid calculation of the collision data
#define VER_REPAIR_STATICMESH_COLLISION						241
//	242 
//	-	Added lighting channel support
#define VER_LIGHTING_CHANNEL_SUPPORT						242
//	243 
//	-	LODGroups for particle systems
#define VER_PARTICLESYSTEM_LODGROUP_SUPPORT					243
//	244
//	-	Changed default volumes for MikeL, propagating to all instances
#define VER_SOUNDNODEWAVE_DEFAULT_CHANGE					244
//	245 
//	-	Added EngineVersion to FPackageFileSummary
#define VER_PACKAGEFILESUMMARY_CHANGE						245
//	246 
//	-	Particles with linear color
#define VER_PARTICLESYSTEM_LINEARCOLOR_SUPPORT				246
//	247 
//	-	Added ExportFlags to FObjectExport
#define VER_FOBJECTEXPORT_EXPORTFLAGS						247
//	248
//	-	Removed COMPONENT_CLASS_BRIDGE code
#define VER_REMOVED_COMPONENT_CLASS_BRIDGE					248
//	249
//	-	Moved ExportMap and ImportMap to beginning of file and extended FPackageFileSummary
#define VER_MOVED_EXPORTIMPORTMAPS_ADDED_TOTALHEADERSIZE	249
//	250
//	-	Introduced concept of default poly flags
#define VER_DEFAULTPOLYFLAGS_CHANGE							250
#define VER_USTRUCT_SERIALIZETAGGEDPROPERTIES_BROKEN		250
//	251
//	-	Changed lazy array serialization
#define VER_LAZYARRAY_SERIALIZATION_CHANGE					251
//	252
//	-	Added USoundNodeMixer::InputVolume
#define VER_USOUNDNODEMIXER_INPUTVOLUME						252
//	253
//	-	Added version number for precooked physics data
#define VER_SAVE_PRECOOK_PHYS_VERSION						253
//	254
//	-	Added compression support to TLazyArray
#define VER_LAZYARRAY_COMPRESSION							254
//	255
//	-	Changed the UI system to use instances of UIState for tracking ui menu states, rather than child classes of UIState
#define VER_CHANGED_UISTATES								255
//	256
//	-	Fixed brush polyflags defaulting to PF_DefaultFlags which should ONLY be the case for surfaces and polys but not for brushes
#define	VER_FIXED_BRUSH_POLYFLAGS							256
//	257
//	-	Made UIAction.ActionMap transient
#define VER_MADE_ACTIONMAP_TRANSIENT						257
//	258
//	-	Safe version for UStruct serialization bug
#define	VER_USTRUCT_SERIALIZETAGGEDPROPERTIES_FIXED			258
//	259
//	-	Added terrain InfoFlags
#define	VER_TERRAIN_ADDING_INFOFLAGS						259
//	260
//	-	Added PayloadFilename to FLazyLoader
#define	VER_LAZYLOADER_PAYLOADFILENAME						260
//	261
//	-	Added CursorMap to UISkin
#define VER_ADDED_CURSOR_MAP								261
//	262
//	-	Static mesh property fixup
#define VER_STATICMESH_PROPERTY_FIXUP						262
//	263
//	-	InfoData in terrains error... wasn't copy-n-pasting correctly so saved maps are botched
#define VER_TERRAIN_INFODATA_ERROR							263
//	264
//	-	NavigationPoints were having their DrawScale incorrectly changed by the editor, so saved maps need to have it reset to the default
#define VER_NAVIGATIONPOINT_DRAWSCALE_FIXUP					264
//	265
//	-	Changed USequenceOp.SeqOpOutputLink.LinkAction from name to object reference
#define VER_CHANGED_LINKACTION_TYPE							265
//	266
//	-	Replaced TLazyArray with FUntypedBulkData
#define	VER_REPLACED_LAZY_ARRAY_WITH_UNTYPED_BULK_DATA		266
//	267
//	-	Components no longer serialize TemplateName unless the component is a template
//	-	Fixed ObjectArchetype for components not always pointing to correct object
#define VER_FIXED_COMPONENT_TEMPLATES						267
//	268
//	-	Static mesh property fixup... again
#define VER_STATICMESH_PROPERTY_FIXUP_2						268
//	269
//	-	Added folder name to UPackage
#define VER_FOLDER_ADDED									269
//	270
//	-	Changed WarGame's ActorFactories to be name only and on save store a reference to the content they
//      need to spawn.  This makes it so we don't have all possible content loaded when only needed a subset
#define VER_WARFARE_FACTORIES_ONLY_REF_WHAT_THEY_NEED		270
//	271
//	-	Fixed 2D array of floats serialized by FStaticMeshTriangleBulkData::SerializeElement for UVs to match
//		the memory layout as required by bulk serialization
#define	VER_FIXED_TRIANGLE_BULK_DATA_SERIALIZATION			271
//	272
//	-	Hardcoded FTerrainMaterialMask bit size
#define	VER_HARDCODED_TERRAIN_MATERIAL_MASK_SIZE			272
//	273
//	-	Removed UPrimitiveComponent->ComponentGuid
#define	VER_REMOVED_COMPONENT_GUID							273
//	274
//	-	Refactored UISkin into two classes (UISkin/UICustomSkin)
#define VER_REFACTORED_UISKIN								274
//	275
//	-	Added level only lighting option, defaults to TRUE for static only lights
#define VER_ADDED_LEVEL_ONLY_LIGHTING_OPTION				275
//	276
//	-	Added platform flags to UClass
#define VER_ADDED_PLATFORM_FLAGS							276
//	277
//	-	Added CookedContentVersion to FPackageFileSummary
#define VER_PACKAGEFILESUMMARY_CHANGE_COOK_VER_ADDED		277
//	278
//	-	Made FLinearColor serialize as a unit
#define VER_CHANGED_FLINEARCOLOR_SERIALIZATION				278
//	279
//	-	Made FLinearColor serialize as a unit
#define VER_MADE_IMMUTABLE_NONINHERIT						279
//  280
//  -   Added per level navigation lists for streaming level fixups
#define VER_PERLEVEL_NAVLIST								280
//	281
//	-	Added SourceStyleID to UIStyle_Combo
#define	VER_ADDED_SOURCESTYLEID								281
//	282
//  -   Integrated FaceFX 1.5 and cleaned up FaceFX serialization
#define VER_FACEFX_1_5_UPGRADE								282
//	283
//  -   Remapped SoundCueLocalized instances to SoundCue
#define VER_REMAPPED_SOUNDCUELOCALIZED_TO_SOUNDCUE			283
//	284
//	-	Added USoundNodeConcatenator::InputVolume
#define VER_USOUNDNODECONCATENATOR_INPUTVOLUME				284
//	285
//	-	Pre-cook physics data for per-triangle static mesh collision.
#define VER_PRECOOK_PERTRI_PHYS_STATICMESH					285
//	286
//	-	Changed FInputKeyAction to have a array of UIAction instead of a single UIAction.
#define VER_INPUTKEYACTION_HAS_ARRAY_OF_UIACTION			286
//	287
//	-	Added UInterpTrackSound::PostLoad for setting the volume and pitch of existing Matinee sound keys.
#define VER_INTERP_TRACK_SOUND_VOLUME_PITCH					287
//	288
//	-	Changed UClass.Interfaces to be a map
#define VER_CHANGED_INTERFACES_TO_MAP						288
//	289
//	-	Added animation compression.
#define VER_ANIMATION_COMPRESSION							289
//	290
//	-	Rewrote simplified collision.
#define VER_NEW_SIMPLE_CONVEX_COLLISION						290
//	291
//	-	Eliminated book-keeping overhead in animation compression.
#define VER_ANIMATION_COMPRESSION_SINGLE_BYTESTREAM			291
//	292
//	-	Operations on UAnimSequence::CompressedByteStream are now aligned to four bytes.
#define VER_ANIMATION_COMPRESSION_FOUR_BYTE_ALIGNED			292
//	293
//	-	Added inclusion/ exclusion volumes to light component
#define VER_ADDED_LIGHT_VOLUME_SUPPORT						293
//	294
//	-	Added permuted data for simplified collision to use SIMD instructions
#define VER_SIMD_SIMPLIFIED_COLLISION_DATA					294
//	295
//	-	Fix distributions on DistanceCrossFade
#define VER_DISTANCE_CROSSFADE_DISTRIBUTIONS_RESET			295
//	296
//	-	Added NameIndexMap to USkeletalMesh to speed up MatchRefBone
#define VER_ADD_SKELMESH_NAMEINDEXMAP						296
//  297
//	-	Rendering refactor
#define VER_RENDERING_REFACTOR								297
//  298
//	-	Terrain material resource serialization
#define VER_TERRAIN_MATERIALRESOURCE_SERIALIZE				298
//  299
//	-	Adding ScreenPositionScaleBiasParameter and SceneDepthCalcParameter 
//		parameters to FMaterialPixelShaderParameters
#define VER_SCREENPOSSCALEBIAS_SCENEDEPTHCALC_PARAMETERS	299
//	300
//	-	Added TwoSidedSignParameter to FMaterialPixelShaderParameters
#define VER_TWOSIDEDSIGN_PARAMETERS							300
//	301
//	-	A checkpoint to force all materials to be recompiled
#define VER_MATERIAL_RECOMPILE_CHECKPOINT					301
//	302
//	-	FDistributions
#define VER_FDISTRIBUTIONS									302
//	303
//	-	Encapsulated UIDockingSet
#define VER_UIDOCKINGSET_CHANGED							303
//	304
//	-	Implemented FalloffExponent in Gemini
#define VER_FALLOFF_EXPONENT_GEMINI							304
//	305
//	-	Remapping TextureRenderTarget to TextureRenderTarget2D
#define VER_REMPAP_TEXTURE_RENDER_TARGET					305
//	306
//	-	Added compilation errors to FMaterial
#define VER_FMATERIAL_COMPILATION_ERRORS					306
//	307
//	-	Added platform mask to shader cache
#define VER_ADDED_PLATFORMTOSERIALIZEMASK					307
//	308
//	-	Forcing all FRawSistributions to be rebuilt
#define VER_FDISTRIBUTION_FORCE_DIRTY						308
//	309
//	-	Enabled single-pass component instancing
#define	VER_SINGLEPASS_COMPONENT_INSTANCING					309
//	310
//	-	Added profiles to AimOffset node
#define	VER_AIMOFFSET_PROFILES								310
//	311
//	-	Added NumInstructions to FShader
#define VER_SHADER_NUMINSTRUCTIONS							311
//	312
//	-	Rewrote decals for the new renderer.
#define VER_DECAL_REFACTOR									312
//	313
//	-	Moved lowest LOD regeneration to PostLoad.
#define VER_EMITTER_LOWEST_LOD_REGENERATION					313
//	314
//	-	Rotated light-map basis to allow seamless mirroring of UVs over texture X axis.
#define VER_LIGHTMAP_SYMMETRIC_OVER_X						314
//	315
//	-	Remove CollisionModel from StaticMesh
#define VER_REMOVE_STATICMESH_COLLISIONMODEL				315
//	316
//	-	Code to convert legacy skylight-primitive interaction semantics to light channels.
#define VER_LEGACY_SKYLIGHT_CHANNELS						316
//	317
//	-	Recompile shaders for depth-bias expression fix.
#define VER_DEPTHBIAS_SHADER_RECOMPILE						317
//	318
//	-	Recompile shaders for vertex light-map fix.
#define VER_VERTEX_LIGHTMAP_SHADER_RECOMPILE				318
// 319
//	-	Recompile global shaders for modulated shadows
#define VER_MOD_SHADOW_SHADER_RECOMPILE						319
//	320
//	-	Recompile emissive shaders for sky light lower hemisphere support.
#define VER_SKYLIGHT_LOWERHEMISPHERE_SHADER_RECOMPILE		320
//	321
//	-	Serialize TTransArray owner
#define	VER_SERIALIZE_TTRANSARRAY_OWNER						321
//	322
//	-	Rewritten package map that can replicate objects by index without linkers
#define VER_LINKERFREE_PACKAGEMAP							322
//	323
//	-	Added gamma correction to simple element pixel shader
#define VER_SIMPLEELEMENTSHADER_GAMMACORRECTION				323
//	324
//	-	Fixed up PreviewLightRadius for PointLightComponents
#define VER_FIXED_POINTLIGHTCOMPONENT_LIGHTRADIUS			324
//	325
//	-	Added ColorScale and OverlayColor to GammaCorrectionPixelShader
#define VER_GAMMACORRECTION_SHADER_RECOMPILE				325
//	326
//	-	Added texture dependency length information to FMaterial
#define VER_MATERIAL_TEXTUREDEPENDENCYLENGTH				326
//	327
//	-	Added cached cooked audio data for Xbox 360
#define VER_ADDED_CACHED_COOKED_XBOX360_DATA				327
//	328
//	-	Added color saturate when doing a pow to the simple/gamma pixel shaders
#define VER_SATURATE_COLOR_SHADER_RECOMPILE					328
//	329
//	-	Changed the DeviceZ to WorldZ conversion shader
#define VER_DEVICEZ_CONVERT_SHADER_RECOMPILE				329
//	330
//	-	Added bIsMasked to UMaterial (affects velocity shaders)
#define VER_MATERIAL_ISMASKED_FLAG							330
//	331
//	-	AimOffset Nodes used Quaterions intead of Rotators
#define VER_AIMOFFSET_ROT2QUAT								331
//	332
//	-	Replaced ULightMap* with FLightMap*
#define VER_LIGHTMAP_NON_UOBJECT							332
//	333 
//	-	TResourceArray usage for mesh rendering data
#define VER_USE_UMA_RESOURCE_ARRAY_MESH_DATA				333
//	334
//	-	Added package compression
#define VER_ADDED_PACKAGE_COMPRESSION_SUPPORT				334
//	335
//	-	Changed terrain shader to only be compiled for terrain materials
#define VER_SHADER_RECOMPILE_FOR_TERRAIN_MATERIALS			335
//	336
//	-	Changed the permuted planes in FConvexVolume to include repeats so that pure SIMD tests can be done
#define VER_CONVEX_VOLUMES_PERMUTED_PLANES_CHANGE			336
//	337
//	-	Removed redudant enums from UIComp_AutoAlign
#define	VER_REMOVED_REDUNDANT_ENUMS							337
//	338
//	-	VelocityShader/MotionBlurShader recompile
#define VER_MOTIONBLURSHADER_RECOMPILE						338
//	339
//	-	Added color exp bias term to simple elemnt pixel shader
#define VER_SIMPLE_ELEMENT_SHADER_RECOMPILE					339
//	340
//	-	Added CompositeDynamic lighting channel
#define VER_COMPOSITEDYNAMIC_LIGHTINGCHANNEL				340
//	341
//	-	Recompile modulated shadow pixel shader
#define VER_MODULATESHADOWPROJECTION_SHADER_RECOMPILE		341
//	342
//	-	Upgrading to July XDK requires recompiling shaders
#define VER_JULY_XDK_UPGRADE								342
//	343
//	-	FName change (splits an FName to name and number pair)
#define VER_FNAME_CHANGE_NAME_SPLIT							343
//	344
//	-	Recompile translucency pixel shader.
#define VER_TRANSLUCENCY_SHADER_RECOMPILE					344
//	345
//	-	Added code to fix PointLightComponents that have an invalid PreviewLightRadius [presumably] resulting from old T3D text being pasted into levels
#define VER_FIXED_POINTLIGHTCOMPONENT_LIGHTRADIUS_AGAIN		345
//	346
//	-	VSM Shadow projection. Recompile shadow projection shaders
#define VER_SHADER_VSM_SHADOW_PROJECTION					346
//	347
//	-	Changed how old name tables are loaded to split the name earlier to reduce extra FNames in memory from old packages
#define VER_NAME_TABLE_LOADING_CHANGE						347
//	348
//	-	Added code to fix PointLightComponents that have an invalid PreviewLightRadius component (isn't the same component as the one contained in the owning actor's components array)
#define VER_FIXED_PLC_LIGHTRADIUS_POST_COMPONENTFIX			348
//  349
//	-	Fix for permuting vertex data for FConvexElem and FConvexVolume
#define VER_FIX_CONVEX_VERTEX_PERMUTE						349
//	350
//	-	Terrain serialize material resource guids
#define VER_TERRAIN_SERIALIZE_MATRES_GUIDS					350
//	351
//	-	USeqAct_Interp now saves the transformations of actors it affects.
#define VER_ADDED_SEQACT_INTERP_SAVEACTORTRANSFORMS			351
//	352
//	-	Emissive shader optimizations require recompiling shaders
#define VER_EMISSIVE_OPTIMIZATIONS_SHADER_RECOMPILE			352
//	353
//	-	Only bloom positive scene color values on PC to mimic XBOX
#define VER_DOFBLOOMGATHER_SHADER_RECOMPILE					353
//	354,355
//	-	Recompile fog shader
#define VER_HEIGHTFOG_SHADER_RECOMPILE						355
//	356
//	-	Recompile VSM filter depth gather shader
#define VER_VSMFILTERGATHER_SHADER_RECOMPILE				356
//	357
//	-	Added a bUsesSceneColor flag to UMaterial
#define VER_MATERIAL_USES_SCENECOLOR_FLAG					357
//	358
//	-	StaticMeshes now have physics cooked data saved in them at defined scales.
#define VER_PRECACHE_STATICMESH_COLLISION					358
//	359
//	-	Terrain packed weight maps
#define VER_TERRAIN_PACKED_WEIGHT_MAPS						359
//	360
//	-	Upgrade to August XDK
#define VER_AUGUST_XDK_UPGRADE								360
//	361
//	-	Force recalculation of the peak active particles
#define VER_RECALC_PEAK_ACTIVE_PARTICLES					361
//	362
//	-	Added setable max bone influences to GPU skin vertex factory and max influences to skel mesh chunks
#define VER_GPUSKIN_MAX_INFLUENCES_OPTIMIZATION				362
//	363
//	-	Added an option to control the depth bias when rendering shadow depths 
#define VER_SHADOW_DEPTH_SHADER_RECOMPILE					363
//	364
//	-	Merge all static mesh vertex data into a single vertex buffer
#define VER_STATICMESH_VERTEXBUFFER_MERGE					364
//	365
//	-	Precomputed 'force stream mips' array in ULevel
#define VER_LEVEL_FORCE_STREAM_TEXTURES						365
//	366
//	-	Recompile FUberPostProcessBlendPixelShader to fix shadow color clamping
#define VER_UBERPOSTPROCESSBLEND_PS_RECOMPILE				366
//	367
//	-	VelocityShader recompile
#define VER_VELOCITYSHADER_RECOMPILE						367
//  368
//	-	Level streaming volume changes
#define VER_ADDED_LEVELSTREAMINGVOLUME_USAGE				368
//	369
//	-	Changed LOADING_COMPRESSION_CHUNK_SIZE from 32K to 128K
#define VER_CHANGED_COMPRESSION_CHUNK_SIZE_TO_128			369
//	370
//	-	Terrain vertex factory shader has been changed to use MulMatrix
#define VER_RECOMPILE_TERRAIN_SHADERS						370
//	371
//	-	RateScale has been added to AnimNodeSynch groups.
#define VER_ANIMNODESYNCH_RATESCALE							371
//	372
//	-	Serialization of the animation compression bytestream now accounts for padding.
#define VER_ANIMATION_COMPRESSION_PADDING_SERIALIZATION		372
//	373
//	-	RateScale has been added to AnimNodeSynch groups.
#define VER_DOFBLOOMGATHER_DISTORTION_SHADER_RECOMPILE		373
//	374
//	-	Added code to fix SpotLightComponents that have an invalid PreviewInnerCone and PreviewOuterCone component (isn't the same component as the one contained in the owning actor's components array)
#define VER_FIXED_SPOTLIGHTCOMPONENTS						374
//	375
//	-	Added UExporter::PreferredFormatIndex.
#define VER_ADDED_UExPORTER_PREFFERED_FORMAT				375

//	376
//	-	Added cached cooked audio data for PS3
#define VER_ADDED_CACHED_COOKED_PS3_DATA					376
//	377
//	-	Added property metadat to UClass
#define VER_ADDED_PROPERTY_METADATA							377
//	378
//	-	Added property metadat to UClass
#define VER_DECAL_STATIC_DECALS_SERIALIZED					378
//	379
//	-	Added Hardware PCF support, requires shadow filter shaders recompile
#define VER_HARDWARE_PCF									379
//  380
//	-	Added cached cooked ogg vorbis data for the PC
#define VER_ADDED_CACHED_COOKED_PC_DATA						380
//  381
//	-	Changed the filter buffer to be fixed point, which allows filtering on all hardware
#define VER_FIXEDPOINT_FILTERBUFFER							381
// 382
//	-	Upgrade to XMA2
#define VER_XMA2_UPGRADE									382
// 383
//	-	Upgrade to XMA2
#define VER_ADDED_RAW_SURROUND_DATA							383
// 384
//	-	Fixed a problem that was causing curves to have incorrect tangents at end points.
#define VER_FIXED_CURVE_INTERP_TANGENTS		                384
// 385
//	-	Fixed versioning problem for SoundNodeWave
#define VER_UPDATED_SOUND_NODE_WAVE			                385
// 386
//	-	Remove RigidBodyIgnorePawns flag.
#define VER_REMOVE_RB_IGNORE_PAWNS							386
// 387
//	-	Fixed a bug with CollisionType handling that caused some actors to have collision incorrectly turned off after being edited
#define VER_COLLISIONTYPE_FIX				                387
//  388
//	-	Fix for using wrong values when indexing into the VertexData array and on the duplicated verts at the end
#define VER_FIX_BAD_INDEX_CONVEX_VERTEX_PERMUTE				388
// 389
//	-	Added lightmap texture coordinates to FDecalVertex.
#define VER_DECAL_ADDED_DECAL_VERTEX_LIGHTMAP_COORD			389
// 390
//	-	Added NumChannels to SoundNodeWave
#define VER_ADDED_NUM_CHANNELS								390
//  391
//	-	Changed terrain to be high-res edit
#define VER_TERRAIN_HIRES_EDIT								391
//	392
//	-	Reversed order of gamma correction and rescaling of vertex light-maps to match texture light-maps.
#define VER_VERTEX_LIGHTMAP_GAMMACORRECTION_FIX				392
//	393
//	-	Reversed order of gamma correction and rescaling of vertex light-maps to match texture light-maps.
#define VER_ADDED_LOOP_INDEFINITELY							393
//  394
//	-	Added texture density material shaders
#define VER_TEXTUREDENSITY									394
//	395
//	-	Added fallbacks for lack of floating point blending support.
#define VER_FP_BLENDING_FALLBACK							395
//	396
//	-	Added CollisionType to ActorFactoryDynamicSM and deprecated collision flags in that class and subclasses
#define VER_ADD_COLLISIONTYPE_TO_ACTORFACTORYDYNAMICSM		396
//	397
//	-	Forcing all FRawSistributions to be rebuilt (fixes very short in time curves, and optimizes 2 keyframe linear 'curves')
#define VER_FDISTRIBUTION_FORCE_DIRTY2						397
//	398
//	-	Added support for decals on GPU-skinned skeletal meshes.
#define VER_DECAL_SKELETAL_MESHES							398
//	399
//	-	Added SP_PCD3D_SM2 shader platform
#define VER_SM2_PLATFORM									399
//	400
//	-	Added support for decals on CPU-skinned skeletal meshes.
#define VER_DECAL_CPU_SKELETAL_MESHES						400
//	401
//	-	Temporarily disabled support for unlit skinned decal materials for want of shader constants.
#define VER_DECAL_DISABLED_UNLIT_MATERIALS_SKELETAL_MESHES	401
//	402
//	-	Removed registers from skiined decal vertex factory; reenabled unlit skinned decal materials.
#define VER_DECAL_REMOVED_VELOCITY_REGISTERS				402
//	402
//	-	Removed registers from skiined decal vertex factory; reenabled unlit skinned decal materials.
//	424
//	-	Fixed UIScrollbars but they all need to be recreated
#define	VER_FIXED_UISCROLLBARS								424
//	429
//	-	Removed redundant UI behavior flag
#define	VER_REMOVED_DISALLOW_REPARENTING_FLAG				429

#define VER_EPIC_MERGE_APR_6								430

#define VER_EPIC_MERGE_APR_26								431

#define VER_BLOOM_MODIFIED									432

#define VER_MEASURE_LUMINANCE_SHADER_REGISTER_CONFIG		433

#define VER_FIX_BLOOM_BURST_BLACK_PIXEL						434

#define VER_SUPPORT_TRANSLUCENT_VIEW						435

//	-	Added support for static mesh vertex colors
#define VER_EPIC_CORE_MERGE_MAY_11							436

// 
#define VER_AVA_ALTER_PACKAGELAYOUT							437

//  466
//  -	Separated static mesh positions into a separate buffer
#define VER_SEPARATED_STATIC_MESH_POSITIONS					438

//  463
//	-	Remove cooked terrain data (using heightfield now)
#define VER_REMOVE_COOKED_PHYS_TERRAIN						439

// !!
// !! NOTE: when adding a new version, change the VER_LATEST_ENGINE macro below. This will avoid needing to change UnObjVer.cpp. 
// !!
#define VER_LATEST_ENGINE									VER_REMOVE_COOKED_PHYS_TERRAIN

// Cooked packages loaded with an older package version are recooked
#define VER_LATEST_COOKED_PACKAGE							43

//!{ 2006-05-08	허 창 민
#define VER_AVA_SAMPLE_AMBIENT								1

#define VER_AVA_UNWRAP_STATICMESH_LIGHTMAP_COORDS			2

#define VER_AVA_REMOVE_SAMPLE_AMBIENT						3

#define VER_AVA_STATICMESH_EXPORT_RADIOSITY_GEOMETRY		4

#define VER_AVA_REALLY_REMOVE_SAMPLE_AMBIENT				5

#define VER_AVA_SHAMBIENT									6	

#define VER_AVA_ENVCUBE										7

#define VER_AVA_ADD_CLUSTERNUMBER_TO_FBSPNODE				8

#define VER_AVA_PVS_RENDERING								9

#define VER_AVA_PVS_RENDERING_PLACE_STATICMESH				10

// - Added viewspace normal material shaders
#define	VER_AVA_VIEWSPACENORMAL								11

// - normal map이 걸려있지 않은 surface에 대한 optimize
#define VER_AVA_NEEDSBUMPEDLIGHTMAP							12

#define VER_AVA_ADD_MATERIAL_FALLBACK						13

#define VER_AVA_REMOVE_LIGHTSCALE							14

#define VER_AVA_ADD_STRUCTURAL_BSP_TO_LEVEL					15

#define VER_AVA_ADD_POOR_SM2								16

#define VER_AVA_DONOTLOAD_NONRELEVANT_TEXTURES				17

#define VER_AVA_ADD_MINLUMINANCE_SHADER_RECOMPILE			18

#define VER_AVA_ADD_TONEMAPTEXTURE_SHADER_RECOMPILE			19

#define VER_AVA_MULTIPLE_AMBIENTCUBEVOLUMES					20

#define VER_AVA_ADD_IRRADIANCE_TO_AMBIENTCUBE				21

#define VER_AVA_COMPRESSED_AMBIENTCUBE						22

#define VER_AVA_OMIT_DEPTH_IN_LDR_COMBINE_PS				23

#define VER_AVA_TWEAK_NO_BUMP_LIGHTMAP						24

#define VER_AVA_TWEAK_SM2_LIGHTING							25

#define VER_AVA_COLORCORRECTION								26

#define VER_AVA_COLORCORRECTION_WORLD						27

#define VER_AVA_IRRADIANCE_ENVMAP							28

#define VER_AVA_SHADER_TRANSFORM_OPTIMIZED					29

#define VER_AVA_SHADER_EMISSIVE_PATH_OPTIMIZED				30

#define VER_AVA_ADD_BUMPONLY_VIEWMODE						31

#define VER_AVA_ADD_CASCADED_SHADOWMAP						32

#define VER_AVA_FILLCOLOR_RECOMPILE							33

#define VER_AVA_CASCADEDSHADOWDEPTH_RECOMPILE				34

#define	VER_AVA_ADD_LEAF_PORTAL_RENDERING					35

// NOTE : when adding a new version, change the VER_LATEST_AVA macro below.
#define VER_LATEST_AVA										VER_AVA_ADD_LEAF_PORTAL_RENDERING
//!} 2006-05-08	허 창 민

// Version access.

extern INT			GEngineVersion;					// Engine build number, for displaying to end users.
extern INT			GBuiltFromChangeList;			// Built from changelist, for displaying to end users.


extern INT			GEngineMinNetVersion;			// Earliest engine build that is network compatible with this one.
extern INT			GEngineNegotiationVersion;		// Base protocol version to negotiate in network play.

extern INT			GPackageFileVersion;			// The current Unrealfile version.
extern INT			GPackageFileMinVersion;			// The earliest file version that can be loaded with complete backward compatibility.
extern INT			GPackageFileLicenseeVersion;	// Licensee Version Number.

extern INT          GPackageFileCookedContentVersion;  // version of the cooked content
