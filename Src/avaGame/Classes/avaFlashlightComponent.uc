class avaFlashlightComponent extends SpotlightComponent native;

var rotator Rotation;

cpptext
{
	virtual void SetTransformedToWorld();
}

native function SetAttachmentInfo(float y, int pitch);