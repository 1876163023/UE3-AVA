class avaKBreakableDynamic extends avaKBreakable native;

defaultproperties
{	
	bNoDelete=false			// bNoDelete �� false �̰� bStatic �� false �̾�߸� avaKBreakable �� Dynamic �ϰ� Spawn �� �� �ִ�.
							// ����, bNoDelete Flag �� Destroy ���� ShutDown �� ���� �Ǵ��ؼ��� �ȵȴ�...
	bDynamicSpawned = true // bDynamicSpawned �� �ʿ��� ��ġ�ߴ����� �Ǵ��Ѵ�.
}