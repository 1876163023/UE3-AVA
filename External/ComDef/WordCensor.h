/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: CHS

	Name: WordCensor.h

	Description: ��Ģ�� �˻�.

***/

#pragma once

class IWordFilter
{
public:
	virtual BOOL Init() = 0;
	virtual void SetIgnoreCh(TCHAR) = 0;			// ���� ���� ���
	virtual void ReplaceIgnoreCh(TCHAR*) = 0;		// ���� ���ڸ� �����Ͽ� String�� �����ش�.

	//BOOL IsDigit(TCHAR ch);				// ���ڰ� �������� Ȯ��
	//BOOL IsEnglish(TCHAR ch);			// ���ڰ� �������� Ȯ��
	//BOOL IsKoreanWansung(TCHAR ch);		// ���ڰ� �ѱ� �ϼ������� Ȯ��
	//BOOL IsJapanese(TCHAR ch);		// ���ڰ� �Ϻ������� Ȯ��
	//BOOL IsAllowed(LPCTSTR str);		// ��� ���ڸ��� ������ �������� Ȯ��: ����, ����, �ѱ� �ϼ���

	virtual void SetCensorWord(TCHAR*) = 0;			// ��Ģ �ܾ� �ε�
	//BOOL IsMatchCurse(LPCTSTR str);		// ��Ģ�� ��ġ �Ǵ�
	virtual BOOL IsIncludeCurse(TCHAR*) = 0;	// ��Ģ�� ���� �Ǵ�
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

