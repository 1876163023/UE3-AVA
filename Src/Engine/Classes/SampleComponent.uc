/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SampleComponent extends PrimitiveComponent
	native
	noexport;

/** These mirror the C++ side properties. I'm making a class here so
    LineBatchComponent will get the defaultprops from the PrimitiveComponent base class */

// Virtual function table.
var	native const noexport pointer FPrimitiveRenderInterfaceVfTable;

var native transient const array<pointer> Samples;
