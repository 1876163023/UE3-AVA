class avaAttachment_BaseLightStick extends avaWeaponAttachment;

var class<PointLightComponent>	LightClass;
var PointLightComponent			Light;


var	bool						bLightOn;

function DetachFrom( avaPawn p )
{
	DisableLight();
	Super.DetachFrom( p );
}

simulated function DisableLight()
{
	if ( Light != None )
	{
		Light.SetEnabled( false );
		bLightOn = false;
	}
}

simulated function EnableLight()
{
	if ( bLightOn == true )	return;	
	bLightOn = true;
	Light = new(Outer) LightClass;
	Instigator.Mesh.AttachComponent( Light, AttachmentBoneName );
}

simulated function ChangeWeaponState( int NewState )
{
	Super.ChangeWeaponState( NewState );
	if ( NewState == 1 )
	{
		EnableLight();
	}
}

defaultproperties
{
	DamageCode				=	Grenade
	bMeshIsSkeletal			=	false
	MeshName				=	"Wp_LightStick.MS_LightStick_EU_3p"
	WeaponClass				=	class'avaWeap_BaseLightStick'
	MuzzleFlashSocket		=	None
	MuzzleFlashPSCTemplate	=	None
	MuzzleFlashDuration		=	0
	AttachmentWeaponType	=	WBT_SMG01
	AnimPrefix				=	LS
	DeathIconStr			=	"T"

	LightClass				=	class'avaLightStickComponent'
}
