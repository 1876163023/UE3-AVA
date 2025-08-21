/*
	잡다한 함수들을 모아둠.

	2007/04/16	고광록
*/
class avaUtil extends Object
	native;


/*! @brief 텍스쳐들을 갱신 또는 생성한다.
	@note
		Texture.UpdateTexture를 호출한다.
*/
static native function UpdateTextures(array<Texture> Textures);

//! appSeconds()함수를 호출해 준다.
static native function float GetSeconds();

defaultproperties
{

}