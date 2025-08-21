#pragma once


typedef TMap<FString, FString> TMAP_PARAM;
typedef TMap<FString, TMAP_PARAM> TMAP_SECTION;

class CIni
{
private:
	TMAP_SECTION sections;

	TMAP_PARAM *pCurrSect;
	INT idCurrSect;
	INT idCurrParam;

	UBOOL bOpen;

	TCHAR cfgfn[256];

public:
	CIni();
	CIni(const TCHAR *fn);
	virtual ~CIni();

	static UBOOL FormatFN(TCHAR *path, const TCHAR *fn);

	UBOOL IsOpen() { return bOpen; }

	void SetFN(const TCHAR *fn);

	UBOOL OpenConfig();
	UBOOL OpenConfigAES();
	UBOOL OpenConfigBuffer(const BYTE *buf, UINT buflen);
	UBOOL OpenConfigFile(const TCHAR *fn);
	UBOOL OpenConfigFileAES(const TCHAR *fn);
	void CloseConfig();

	INT GetSectionCount() { return sections.Num(); }
	INT GetParamCount(const TCHAR *section);
	UBOOL GetValue(const TCHAR *section, const TCHAR *key, TCHAR *value);
	FString GetString(const TCHAR *section, const TCHAR *key, const TCHAR *def);
	INT GetInteger(const TCHAR *section, const TCHAR *key, INT def);

	UBOOL SelectSection(const TCHAR *section);
	UBOOL SelectFirstSection();
	UBOOL SelectNextSection();
	UBOOL GetCurrentSectionName(TCHAR *section);
	const TCHAR *GetCurrentSectionName();
	UBOOL IsSectionName(const TCHAR *section);

	UBOOL GetCurrentParam(TCHAR *key, TCHAR *value);
	UBOOL GetValue(const TCHAR *key, TCHAR *value);
	FString GetString(const TCHAR *key, const TCHAR *def);	
	INT GetInteger(const TCHAR *key, INT def);
};

