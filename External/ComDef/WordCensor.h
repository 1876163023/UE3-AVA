/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: CHS

	Name: WordCensor.h

	Description: 금칙어 검사.

***/

#pragma once

class IWordFilter
{
public:
	virtual BOOL Init() = 0;
	virtual void SetIgnoreCh(TCHAR) = 0;			// 무시 문자 등록
	virtual void ReplaceIgnoreCh(TCHAR*) = 0;		// 무시 문자를 제거하여 String을 돌려준다.

	//BOOL IsDigit(TCHAR ch);				// 문자가 숫자인지 확인
	//BOOL IsEnglish(TCHAR ch);			// 문자가 영문인지 확인
	//BOOL IsKoreanWansung(TCHAR ch);		// 문자가 한글 완성형인지 확인
	//BOOL IsJapanese(TCHAR ch);		// 문자가 일본어인지 확인
	//BOOL IsAllowed(LPCTSTR str);		// 허용 문자만을 포함한 문장인지 확인: 숫자, 영문, 한글 완성형

	virtual void SetCensorWord(TCHAR*) = 0;			// 금칙 단어 로딩
	//BOOL IsMatchCurse(LPCTSTR str);		// 금칙어 일치 판단
	virtual BOOL IsIncludeCurse(TCHAR*) = 0;	// 금칙어 포함 판단
	virtual BOOL ReplaceCensorWord(TCHAR*) = 0;
};

class CWordCensor
{	
public:
	CWordCensor(IWordFilter *pCharacterName = NULL, IWordFilter *pChatRoomName = NULL, BOOL bAutoInit = TRUE);
	virtual ~CWordCensor();	

	void SetFilter(IWordFilter *pCharacterName, IWordFilter *pChatRoomName);
	IWordFilter* GetCharacterNameFilter() const { return filterCharacterName; }
	IWordFilter* GetChatRoomNameFilter() const { return filterChatRoomName; }

	BOOL Init();

	BOOL CheckCharacterName(TCHAR *str);
	BOOL CheckRoomName(TCHAR *str);
	BOOL ReplaceChatMsg(TCHAR *str);

	BOOL IsInit() const { return bInit; }

	static BOOL CheckSpecialCharacterInNickName(TCHAR *str);
	static BOOL CheckSpecialCharacterInRoomName(TCHAR *str);

private:
	BOOL bInit;
	IWordFilter* filterCharacterName;
	IWordFilter* filterChatRoomName;
};

