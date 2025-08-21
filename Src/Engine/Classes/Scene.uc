//=============================================================================
// Scene - script exposed scene enums
// Copyright 2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class Scene extends Object
	native(Scene);

/**
 * A priority for sorting scene elements by depth.
 * Elements with higher priority occlude elements with lower priority, disregarding distance.
 */
enum ESceneDepthPriorityGroup
{
	// unreal ed background scene DGP
	SDPG_UnrealEdBackground,
	// world scene DPG
	SDPG_World,
	// foreground scene DPG
	SDPG_Foreground,
	// unreal ed scene DPG
	SDPG_UnrealEdForeground,
	// after all scene rendering
	SDPG_PostProcess,
	SDPG_SkyLayer0,
	SDPG_SkyLayer1,
	SDPG_SkyLayer2,
	SDPG_SkyLayer3,
	///@ava specific
	// 2006/8/21. chnagmin.
	// foreforeground scene DPG for 3d ui
	SDPG_ForeForeground
	///@ava
};

/** bits needed to store DPG value */
//const SDPG_NumBits = 3;
//@ava specific
// 2006/8/21. changmin
const SDPG_NumBits = 4;
