/* 
	this class is expired
*/
class avaSeqAct_DemoPawn extends SequenceAction;

//var() bool							bDemo;
//var() class<avaWeaponAttachment>	WeaponAttachmentClass;
//var() bool							bNoDamage;
//
//event Activated()
//{
//	local avaCharacter p;
//	local SeqVar_Object ObjVar;
//
//	foreach LinkedVariables(class'SeqVar_Object', ObjVar, "Target")
//	{
//		// find the object to change the team of
//		p = avaCharacter(ObjVar.GetObjectValue());
//		if (p != None) 
//		{
//			p.Demo_Active = bDemo;			
//			p.Demo_WeaponAttachmentClass = WeaponAttachmentClass;
//			p.Demo_bNoDamage = bNoDamage;
//
//			p.NotifyTeamChanged();
//		}
//	}
//}
//
//defaultproperties
//{
//	ObjCategory="avaPawn"
//	ObjName="Set DemoPawn parameters"
//
//	bDemo=true
//} 