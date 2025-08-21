/*=============================================================================
  avaAnimBlendByExclusiveAnim
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/06/12 by OZ
		
		�� node �� ����ü �и� Animation �� �ƴ� �������� Animation �� Play �ϵ��� �Ѵ�.
		��ȭ�⸦ ��ġ�� ���, ��ġ�� ��ȭ�⸦ ���� ��쿡 �ش��Ѵ�.
		(�������� ���ϴ� ���...)
		
=============================================================================*/
class avaAnimBlendByExclusiveAnim extends avaAnimBlendBase
	native;

cpptext
{
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
}

defaultproperties
{
	Children(0)=(Name="None Exc",Weight=1.0)
	Children(1)=(Name="Install")
	Children(2)=(Name="Fixed")
	bFixNumChildren=true
}
