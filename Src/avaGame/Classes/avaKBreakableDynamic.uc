class avaKBreakableDynamic extends avaKBreakable native;

defaultproperties
{	
	bNoDelete=false			// bNoDelete 가 false 이고 bStatic 이 false 이어야만 avaKBreakable 을 Dynamic 하게 Spawn 할 수 있다.
							// 따라서, bNoDelete Flag 로 Destroy 할지 ShutDown 을 할지 판단해서는 안된다...
	bDynamicSpawned = true // bDynamicSpawned 로 맵에서 배치했는지를 판단한다.
}