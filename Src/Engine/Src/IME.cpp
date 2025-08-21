#include "EnginePrivate.h"

IMPLEMENT_CLASS(UIMEComposition);

UBOOL UIMEComposition::StaticNotifyAction( BYTE Action, INT Index, INT Value )
{
	UBOOL bResult = FALSE;
#ifdef WITH_IME
	extern UBOOL NotifyIME( BYTE Action, INT Index, INT Value );
	bResult = NotifyIME(Action, Index, Value);
#endif
	return bResult;
}

UBOOL UIMEComposition::NotifyAction( BYTE Action, INT Index, INT Value )
{
	return UIMEComposition::StaticNotifyAction( Action, Index, Value );
}


BYTE UIMEComposition::StaticGetImeLanguage()
{
#ifdef WITH_IME
	extern BYTE GKeyboardInputLanguage;
	return GKeyboardInputLanguage;
#else
	return KBDINPUTLANG_Default;
#endif
}

INT UIMEComposition::StaticGetImeProperty()
{
#ifdef WITH_IME
	extern DWORD ImeProperty;
	return ImeProperty;
#else
	return 0;
#endif
}

/**
* IME에서 현재 사용중인 언어를 확인
*
* @return	현재의 IME 입력 언어를 EKBDINPUTLANGType으로 반환 ( 중국어, 한국어, 일본어 )
*/
BYTE UIMEComposition::GetImeLanguage()
{
	return StaticGetImeLanguage();
}

INT UIMEComposition::GetImeProperty()
{
	return StaticGetImeProperty();
}