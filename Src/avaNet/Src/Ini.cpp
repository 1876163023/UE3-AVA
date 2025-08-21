#include "avaNet.h"
#include "Ini.h"
#include "ComDef/Version.h"
#include "ComDef/avaAES.h"

#pragma warning(disable:4127)


////////////////////////////////////////////////////////////////////////////////////////////////////
// CIni

CIni::CIni() : bOpen(FALSE), pCurrSect(NULL), idCurrSect(-1), idCurrParam(-1)
{
	cfgfn[0] = 0;
}

CIni::CIni(const TCHAR *fn)
{
	CIni();
	_tcscpy(cfgfn, fn);
}

CIni::~CIni()
{
	CloseConfig();
}

UBOOL CIni::FormatFN(TCHAR *path, const TCHAR *fn)
{
	if (!path || !fn)
		return FALSE;
	INT _l = _tcslen(fn);
	if (_l == 0)
		return FALSE;

	if (fn[0] == TEXT('\\') || _tcsstr(fn, TEXT(":\\")))
		_tcscpy(path,fn);
	else
	{
		TCHAR exe[MAX_PATH];
		if (::GetModuleFileName(NULL, exe, 1024) == 0)
		{
			return FALSE;
		}

		TCHAR Drive[_MAX_DRIVE];
		TCHAR Path[_MAX_PATH];
		TCHAR Filename[_MAX_FNAME];
		TCHAR Ext[_MAX_EXT];

		_tsplitpath(exe, Drive, Path, Filename, Ext);

		_stprintf(path,TEXT("%s\\%s\\%s"),Drive,Path,fn);		
	}

	return (_tcslen(path) > 0);
}

void CIni::SetFN(const TCHAR *fn)
{
	//ASSERT(_tcslen(fn) < 256);
	_tcscpy(cfgfn, fn);
}

UBOOL CIni::OpenConfig()
{
	return OpenConfigFile(cfgfn);
}

UBOOL CIni::OpenConfigAES()
{
	return OpenConfigFileAES(cfgfn);
}

#define MAX_CHAR_PER_LINE 2048
#define SECT_GENERAL TEXT("General")

UBOOL CIni::OpenConfigBuffer(const BYTE *buf, UINT buflen)
{
	if (buflen == 0)
		return FALSE;

	char *_tbuf = (char*)buf;
	TMAP_PARAM *_dec = NULL;

	char *_line = strtok(_tbuf, "\r\n");
	TCHAR _tline[1024];
	while ( _line )
	{
		do
		{
			if (*_line == ';' || *_line == '#' || *_line == '\n' || *_line == '\r')
				break;

#ifdef _UNICODE
			INT _ret = MultiByteToWideChar(CP_ACP, 0, _line, (int)strlen(_line), _tline, 1024);
			_tline[_ret] = NULL;
#else
			_tcscpy(_tline, _line);
#endif

			FString _strl = _tline;
			_strl.Trim();
			_strl.TrimTrailing();
			if (_strl.Len() == 0)
				break;

			if (_strl[0] == TEXT('['))
			{
				// section
				UINT _rbracket = _strl.InStr(TEXT("]"), TRUE);
				if (_rbracket == -1)
					break;
				FString _sect = _strl.Mid(1, _rbracket - 1);
				_sect.Trim();
				_sect.TrimTrailing();
				if (_sect.Len() == 0)
					break;
				_sect = _sect.ToUpper();

				if ( !sections.HasKey(_sect) )
				{
					_dec = &sections.Set(*_sect, TMAP_PARAM());
				}
			}
			else
			{
				// config
				UINT _delpos = _strl.InStr(TEXT("="));
				if (_delpos == -1)
					break;

				FString _key = _strl.Left(_delpos);
				_key.Trim();
				_key.TrimTrailing();
				if (_key.Len() == 0)
					break;

				FString _val = _strl.Right(_strl.Len() - _delpos - 1);
				_val.Trim();
				_val.TrimTrailing();
				if (_val.Len() == 0)
					break;

				if (sections.Num() == 0)
				{
					_dec = &sections.Set(SECT_GENERAL, TMAP_PARAM());
				}
				if (!_dec)
				{
					_dec = sections.Find(SECT_GENERAL);
					if (!_dec)
						return FALSE;
				}

				_dec->Set(*_key, *_val);
			}
		}
		while (0);

		_line = strtok(NULL, "\r\n");
	}

	bOpen = TRUE;
	return TRUE;
}

UBOOL CIni::OpenConfigFile(const TCHAR *fn)
{
	TCHAR _path[MAX_PATH];
	if ( !FormatFN(_path, fn) )
		return FALSE;

	FILE *_fp = _tfopen(_path, TEXT("rb"));
	if (!_fp)
		return FALSE;

	fseek(_fp, 0, SEEK_END);
	long _readfilelen = ftell(_fp);
	fseek(_fp, 0, SEEK_SET);

	if (_readfilelen == 0)
		return FALSE;

	BYTE *_buf = new BYTE[_readfilelen];
	memset(_buf, 0, _readfilelen);

	size_t _readbuflen = fread(_buf, sizeof(BYTE), _readfilelen, _fp);
	fclose(_fp);

	UBOOL _res = OpenConfigBuffer(_buf, _readbuflen);
	delete[] _buf;
	return _res;
}

UBOOL CIni::OpenConfigFileAES(const TCHAR *fn)
{
	TCHAR _path[MAX_PATH];
	if ( !FormatFN(_path, fn) )
		return FALSE;

	FILE *_fp = _tfopen(_path, TEXT("rb"));
	if (!_fp)
		return FALSE;

	fseek(_fp, 0, SEEK_END);
	long _readfilelen = ftell(_fp);
	fseek(_fp, 0, SEEK_SET);

	if (_readfilelen == 0)
		return FALSE;

	BYTE *_buf = new BYTE[_readfilelen];
	memset(_buf, 0, _readfilelen);

	size_t _readbuflen = fread(_buf, sizeof(BYTE), _readfilelen, _fp);
	fclose(_fp);

	UBOOL _res = true;
	do
	{
		if (_readbuflen == 0)
		{
			_res = FALSE;
			break;
		}

		CavaAES _aes;
		UINT _buflen = *((UINT*)_buf);
		UINT _cipherlen;
		BYTE _key[17];

		memset(_key, 0, 17);
		_aes.BuildKey(_key, 16, Def::VERSION_ENCRYPT_KEY);

		if ( !_aes.Init((const LPBYTE)_key, 16) )
		{
			_res = FALSE;
			break;
		}

		if ( !_aes.Decrypt(_buf + 4, _readbuflen - 4, _buf, &_cipherlen) )
		{
			_res = FALSE;
			break;
		}
		if (_cipherlen > _readbuflen - 4 || _buflen > _cipherlen)
		{
			_res = FALSE;
			break;
		}
		if (_buflen < _readbuflen)
		{
			memset(_buf + _buflen, 0, _readbuflen - _buflen);
		}

		_res = OpenConfigBuffer(_buf, _buflen);
	}
	while (0);

	delete[] _buf;
	return _res;
}

void CIni::CloseConfig()
{
	if (sections.Num() > 0)
		sections.Empty();
	pCurrSect = NULL;
	idCurrSect = -1;
	idCurrParam = -1;
	bOpen = FALSE;
}

INT CIni::GetParamCount(const TCHAR *section)
{
	if (!bOpen)
		return 0;

	TMAP_PARAM *pSect = sections.Find(FString(section).ToUpper());

	return pSect ? pSect->Num() : -1;
}


UBOOL CIni::GetValue(const TCHAR *section, const TCHAR *key, TCHAR *value)
{
	if (!bOpen)
		return FALSE;

	TMAP_PARAM *pSect = sections.Find( FString(section).ToUpper() );
	if (!pSect)
		return FALSE;

	FString *pVal = pSect->Find( FString(key).ToUpper() );

	if (!pVal)
		return FALSE;

	_tcscpy(value, **pVal);
	return TRUE;
}


FString CIni::GetString(const TCHAR *section, const TCHAR *key, const TCHAR *def)
{
	TCHAR _val[2048];

	if (GetValue(section, key, _val))
		return _val;
	else
		return def;
}

INT CIni::GetInteger(const TCHAR *section, const TCHAR *key, INT def)
{
	TCHAR _val[100];

	if (GetValue(section, key, _val))
		return _tstoi(_val);
	else
		return def;
}


UBOOL CIni::SelectSection(const TCHAR *section)
{
	if (!bOpen)
		return FALSE;

	if ( sections.Num() == 0 )
		return FALSE;

	FString _sect(section);
	_sect = _sect.ToUpper();

	INT i = 0;
	for (TMAP_SECTION::TIterator it(sections); it; ++i, ++it)
	{
		if (it.Key() == _sect)
		{
			pCurrSect = &it.Value();
			idCurrSect = i;
			idCurrParam = (pCurrSect->Num() > 0 ? 0 : -1);
			return TRUE;
		}
	}

	return FALSE;
}


UBOOL CIni::SelectFirstSection()
{
	if (!bOpen)
		return FALSE;

	if ( sections.Num() == 0 )
		return FALSE;

	pCurrSect = &TMAP_SECTION::TIterator(sections).Value();
	idCurrSect = 0;
	idCurrParam = (pCurrSect->Num() > 0 ? 0 : -1);

	return true;
}


UBOOL CIni::SelectNextSection()
{
	if (!bOpen)
		return FALSE;

	if (!pCurrSect || idCurrSect == -1)
		return FALSE;

	++idCurrSect;

	INT i = 0;
	for (TMAP_SECTION::TIterator it(sections); it; ++i, ++it)
	{
		if (i == idCurrSect)
		{
			pCurrSect = &it.Value();
			idCurrParam = (pCurrSect->Num() > 0 ? 0 : -1);
			return TRUE;
		}
	}

	pCurrSect = NULL;
	idCurrSect = -1;
	return FALSE;
}


UBOOL CIni::GetCurrentSectionName(TCHAR *section)
{
	if (!bOpen || !section)
		return FALSE;

	if (!pCurrSect || idCurrSect == -1)
		return FALSE;

	INT i = 0;
	for (TMAP_SECTION::TIterator it(sections); it; ++i, ++it)
	{
		if (i == idCurrSect)
		{
			_tcscpy(section, *it.Key());
			return TRUE;
		}
	}

	return FALSE;
}


const TCHAR *CIni::GetCurrentSectionName()
{
	if (!bOpen)
		return NULL;

	if (!pCurrSect || idCurrSect == -1)
		return FALSE;

	INT i = 0;
	for (TMAP_SECTION::TIterator it(sections); it; ++i, ++it)
	{
		if (i == idCurrSect)
		{
			return *it.Key();
		}
	}

	return NULL;
}


UBOOL CIni::IsSectionName(const TCHAR *section)
{
	if (!bOpen || !section)
		return FALSE;

	if (!pCurrSect || idCurrSect == -1)
		return FALSE;

	INT i = 0;
	for (TMAP_SECTION::TIterator it(sections); it; ++i, ++it)
	{
		if (i == idCurrSect)
		{
			return it.Key() == section;
		}
	}

	return FALSE;
}


UBOOL CIni::GetCurrentParam(TCHAR *key, TCHAR *value)
{
	if (!bOpen || !key || !value)
		return FALSE;

	if (!pCurrSect || idCurrSect == -1 || idCurrParam == -1)
		return FALSE;

	INT i = 0;
	for (TMAP_PARAM::TIterator it(*pCurrSect); it; ++i, ++it)
	{
		if (i == idCurrParam)
		{
			_tcscpy(key, *it.Key());
			_tcscpy(value, *it.Value());
			return TRUE;
		}
	}

	return FALSE;
}


UBOOL CIni::GetValue(const TCHAR *key, TCHAR *value)
{
	if (!bOpen || !key || !value)
		return FALSE;

	if (!pCurrSect || idCurrSect == -1)
		return FALSE;

	FString *pVal = pCurrSect->Find(key);
	if (pVal)
	{
		_tcscpy(value, **pVal);
		return TRUE;
	}

	return FALSE;
}

FString CIni::GetString(const TCHAR *key, const TCHAR *def)
{
	TCHAR _val[2048];

	if (GetValue(key, _val))
		return _val;
	else
		return def;
}

INT CIni::GetInteger(const TCHAR *key, INT def)
{
	TCHAR _val[100];

	if (GetValue(key, _val))
		return _tstoi(_val);
	else
		return def;
}

