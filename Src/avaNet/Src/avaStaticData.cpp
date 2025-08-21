#include "avaNet.h"
#include "avaStaticData.h"
#include "Ini.h"
#include "avaNetClient.h"
#include "ComDef/avaAES.h"
#include "ComDef/Version.h"



UBOOL MakeFullPath(TCHAR *path, TCHAR *filename)
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

	_stprintf(path,_T("%s%s%s"),Drive,Path,filename);

	return TRUE;
}

void Trim(TCHAR* str)
{
	int _len = _tcslen(str);
	if (_len == 0)
		return;

	// trim left
	int _lpos, _rpos;
	for (_lpos = 0; _lpos < _len; _lpos++)
		if (str[_lpos] != TEXT(' ') && str[_lpos] != TEXT('\t') && str[_lpos] != TEXT('\n') && str[_lpos] != TEXT('\r'))
			break;
	if (_lpos == _len)
	{
		str = TEXT("");
		return;
	}

	// trim right
	for (_rpos = _len - 1; _rpos >= 0; _rpos--)
		if (str[_rpos] != TEXT(' ') && str[_rpos] != TEXT('\t') && str[_rpos] != TEXT('\n') && str[_lpos] != TEXT('\r'))
			break;
	if (_rpos == -1)
	{
		str = TEXT("");
		return;
	}
	
	//_tcsncpy(str,str[_lpos],_rpos - _lpos + 1);
	if (_lpos > 0 || _rpos < _len - 1)
	{
		//TCHAR _temp[32];
		_tcsncpy(str,&str[_lpos],_rpos - _lpos + 1);
		str[_rpos - _lpos + 1] = 0;
		//_tcscpy(str,_temp);
	}
}

INT LoadStringsFromFile(TArray<FString> &OutArray, TCHAR *FileName, UBOOL bEncrypted)
{
	if (FileName == NULL || appStrlen(FileName) == 0)
		return 0;

	TCHAR Path[MAX_PATH+1];
	if ( !MakeFullPath(Path, FileName) )
		return 0;

	if (OutArray.Num() > 0)
		OutArray.Empty();

	CavaAES _aes;
	BYTE _key[17];

	if (bEncrypted)
	{
		memset(_key, 0, 17);
		_aes.BuildKey(_key, 16, Def::VERSION_ENCRYPT_KEY);

		if ( !_aes.Init((const LPBYTE)_key, 16) )
		{
			return 0;
		}
	}

	FILE *_fp = _tfopen(Path, TEXT("rb"));
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

	BOOL _res = TRUE;
	do
	{
		if (_readbuflen == 0)
		{
			_res = FALSE;
			break;
		}

		if (bEncrypted)
		{
			UINT _buflen = *((UINT*)_buf);
			UINT _cipherlen;

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
		}

		TCHAR _tline[1024];
		char *_line = strtok((char*)_buf, "\r\n");
		while ( _line )
		{
			do
			{
				if (*_line == '\0' || *_line == '\r' || *_line == '\n')
					break;

#ifdef _UNICODE
				int _ret = MultiByteToWideChar(CP_ACP, 0, _line, (int)strlen(_line), _tline, 1024);
				_tline[_ret] = NULL;
#else
				_tcscpy(_tline, _line);
#endif

				Trim(_tline);
				if(_tcslen(_tline) == 0)
					break;

				_tcsupr(_tline);

				OutArray.Push(_tline);
			}
			while (0);

			_line = strtok(NULL, "\r\n");
		}
	}
	while (0);

	delete [] _buf;

	if (!_res)
		return _res;

	return OutArray.Num();
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemDesc


CUSTOM_SLOT_IDX	CustomStrToValue(TCHAR*	str)
{
	if(!str) return	_CSI_NONE;

	if (!_tcscmp(str,TEXT("F"))) return _CSI_FRONT;
	else if	(!_tcscmp(str,TEXT("M")))	return _CSI_MOUNT;
	else if	(!_tcscmp(str,TEXT("B")))	return _CSI_BARREL;
	else if	(!_tcscmp(str,TEXT("T")))	return _CSI_TRIGGER;
	else if	(!_tcscmp(str,TEXT("G")))	return _CSI_GRIP;
	else if	(!_tcscmp(str,TEXT("S")))	return _CSI_STOCK;
	else return	_CSI_NONE;
}

ITEM_EFFECT_TYPE EffectStrToValue(TCHAR *str)
{
	if(!str) return _IET_NONE;

	if (!_tcscmp(str,TEXT("GR"))) return _IET_GR;
	else if	(!_tcscmp(str,TEXT("EE")))	return _IET_EXP_BOOST;
	else if	(!_tcscmp(str,TEXT("ES")))	return _IET_SP_BOOST;
	else if	(!_tcscmp(str,TEXT("EM")))	return _IET_MONEY_BOOST;
	else return _IET_NONE;
}

DWORD EquipStrToValue(TCHAR* str)
{
	if(!str) return	_EP_NONE;

	if (!_tcscmp(str,TEXT("H1")))	return _EP_H1;
	else if	(!_tcscmp(str,TEXT("H1-1"))) return _EP_H11;
	else if	(!_tcscmp(str,TEXT("H1-2"))) return _EP_H12;
	else if	(!_tcscmp(str,TEXT("H2"))) return	_EP_H2;
	else if	(!_tcscmp(str,TEXT("H3"))) return	_EP_H3;
	else if	(!_tcscmp(str,TEXT("C1"))) return	_EP_C1;
	else if	(!_tcscmp(str,TEXT("C2"))) return	_EP_C2;
	else if	(!_tcscmp(str,TEXT("A1"))) return	_EP_A1;
	else if	(!_tcscmp(str,TEXT("A2"))) return	_EP_A2;
	else if	(!_tcscmp(str,TEXT("B1"))) return	_EP_B1;
	else if	(!_tcscmp(str,TEXT("B3"))) return	_EP_B3;
	else if	(!_tcscmp(str,TEXT("W1"))) return	_EP_W1;
	else if	(!_tcscmp(str,TEXT("W2"))) return	_EP_W2;
	else if	(!_tcscmp(str,TEXT("W3"))) return	_EP_W3;
	else if	(!_tcscmp(str,TEXT("T1"))) return	_EP_T1;
	else if	(!_tcscmp(str,TEXT("T2"))) return	_EP_T2;
	else if	(!_tcscmp(str,TEXT("E")))	return _EP_E;
	else if	(!_tcscmp(str,TEXT("G")))	return _EP_G;
	else if	(!_tcscmp(str,TEXT("K")))	return _EP_K;
	else if	(!_tcscmp(str,TEXT("BT"))) return	_EP_BT;
	else if	(!_tcscmp(str,TEXT("BD"))) return	_EP_BD;
	else if	(!_tcscmp(str,TEXT("R1"))) return	_EP_R1;
	else if	(!_tcscmp(str,TEXT("P1"))) return	_EP_P1;
	else if	(!_tcscmp(str,TEXT("S1"))) return	_EP_S1;
	else if	(!_tcscmp(str,TEXT("R2"))) return	_EP_R2;
	else if	(!_tcscmp(str,TEXT("P2"))) return	_EP_P2;
	else if	(!_tcscmp(str,TEXT("S2"))) return	_EP_S2;
	else if	(!_tcscmp(str,TEXT("R3"))) return	_EP_R3;
	else if	(!_tcscmp(str,TEXT("P3"))) return	_EP_P3;
	else if	(!_tcscmp(str,TEXT("S3"))) return	_EP_S3;
	else if	(!_tcscmp(str,TEXT("R4"))) return	_EP_R4;
	else if	(!_tcscmp(str,TEXT("P4"))) return	_EP_P4;
	else if	(!_tcscmp(str,TEXT("S4"))) return	_EP_S4;
	else if	(!_tcscmp(str,TEXT("FACE"))) return _EP_FACE;
	else if	(!_tcscmp(str,TEXT("EE"))) return _EP_EFF_EE;
	else if	(!_tcscmp(str,TEXT("ES"))) return _EP_EFF_ES;
	else if	(!_tcscmp(str,TEXT("EM"))) return _EP_EFF_EM;
	else return	_EP_NONE;
}



CavaItemDescData::CavaItemDescData()
{
}

CavaItemDescData::~CavaItemDescData()
{
	Final();
}

#define _CHECK_COND(b, v)												\
	if ( !(b) )															\
	{																	\
		_LOG(TEXT("Error! Invalid value (%s); section [%s]\n"), TEXT(#v), _ini.GetCurrentSectionName());	\
		return FALSE;													\
	}

#define _READ_VAL_INT(_d, _k, _c, _n)									\
	if (_ini.GetValue(_n, szTemp) && appStrlen(szTemp) > 0)				\
	{																	\
		(_d)->_k = (_c)appAtoi(szTemp);									\
	}

#define _READ_VAL_FUNC(_d, _k, _f, _n)									\
	if (_ini.GetValue(_n, szTemp) && appStrlen(szTemp) > 0)				\
	{																	\
		(_d)->_k = _f(szTemp);											\
	}

BOOL CavaItemDescData::Init()
{
	if (bInit)
		return FALSE;

	const BOOL bEncrypted =	TRUE;
	TCHAR szTemp[1024];

	// ITEM	DESC Load...
	{
		FString	ItemDescName;
		if ( !GConfig->GetString(CFG_SECTION, CFG_ITEMDESC, ItemDescName, GNetIni) || ItemDescName == TEXT("") )
			ItemDescName = TEXT("ItemDesc.ini");

		CIni _ini(*ItemDescName);

		UBOOL Res = (bEncrypted ? _ini.OpenConfigAES() : _ini.OpenConfig());
		if (Res)
			Res = _ini.SelectFirstSection();

		if(!Res)
		{
			_LOG(TEXT("Error! Failed to load item list; file = %s"), *ItemDescName);
			return FALSE;
		}

		TMap<INT, INT> NPList;
		for (INT i = 0; i < _ini.GetSectionCount(); ++i)
		{
			NPList.Set(i, i);
		}

		INT PrevNPListNum = NPList.Num();
		INT Idx = 0;
		while (NPList.Num() > 0)
		{
			//_LOG(TEXT("[%d][%s]"), Idx, _ini.GetCurrentSectionName());

			BREAK_SECTION_BEGIN()
			{
				if (!NPList.HasKey(Idx))
				{
					break;
				}

				// 기본 데이터
				TID_ITEM _id = (TID_ITEM)_ini.GetInteger(TEXT("Item_ID"),ID_INVALID_ITEM);
				if (_id	== ID_INVALID_ITEM)
				{
					_LOG(TEXT("Error! Failed to read Item_ID from section [%s]"), _ini.GetCurrentSectionName());
					return FALSE;
				}

				if (IsFace(_id))
				{
					// 얼굴
					if (!faceList.ContainsItem(_id))
						faceList.Push(_id);
					NPList.Remove(Idx);
					break;
				}

				TID_ITEM _idParent = (TID_ITEM)_ini.GetInteger(TEXT("InheritFrom"), ID_INVALID_ITEM);
				DWORD _sales_id = (DWORD)_ini.GetInteger(TEXT("Sales_ID"), 0);

				BASE_ITEM_DESC *_desc = NULL;

				if (IsWeaponItem(_id) || IsEquipItem(_id))
				{
					// 무기/장비 아이템 생성
					if (itemList.HasKey(_id))
					{
						_LOG(TEXT("Item [%d] is duplicated"), _id);
						return FALSE;
					}

					FavaItemDesc *_parent = NULL;
					if (_idParent != ID_INVALID_ITEM)
					{
						// 상속 받을 아이템 검색
						_parent = itemList.FindRef(_idParent);
						if (!_parent)
						{
							// 상속 받을 아이템이 아직 처리되지 않았음. 일단 다음으로 넘김.
							_LOG(TEXT("Item [%d]'s parent [%d] is not processed yet; Idx = %d, NP = %d"), _id, _idParent, Idx, NPList.Num());
							break;
						}
					}

					FavaItemDesc *_newDesc = new FavaItemDesc(_id, _parent, _sales_id);
					check(_newDesc);
					itemList.Set(_id, _newDesc);
					itemArray.Push(_newDesc);

					_desc = _newDesc;
				}
				else if (IsCustomItem(_id))
				{
					// 커스텀 아이템 생성
					if (customItemList.HasKey(_id))
					{
						_LOG(TEXT("Custom item [%d] is duplicated"), _id);
						return FALSE;
					}

					FavaCustomItemDesc *_parent = NULL;
					if (_idParent != ID_INVALID_ITEM)
					{
						// 상속 받을 아이템 검색
						_parent = customItemList.FindRef(_idParent);
						if (!_parent)
						{
							// 상속 받을 아이템이 아직 처리되지 않았음. 일단 다음으로 넘김.
							_LOG(TEXT("Custom item [%d]'s parent [%d] is not processed yet; Idx = %d, NP = %d"), _id, _idParent, Idx, NPList.Num());
							break;
						}
					}

					FavaCustomItemDesc *_newDesc = new FavaCustomItemDesc(_id, _parent, _sales_id);
					check(_newDesc);
					customItemList.Set(_id, _newDesc);
					customItemArray.Push(_newDesc);

					_desc = _newDesc;
				}
				else if (IsEffectItem(_id))
				{
					// 효과(캐쉬) 아이템 생성
					if (effectItemList.HasKey(_id))
					{
						_LOG(TEXT("Effect item [%d] is duplicated"), _id);
						return FALSE;
					}

					FavaEffectItemDesc *_parent = NULL;
					if (_idParent != ID_INVALID_ITEM)
					{
						// 상속 받을 아이템 검색
						_parent = effectItemList.FindRef(_idParent);
						if (!_parent)
						{
							// 상속 받을 아이템이 아직 처리되지 않았음. 일단 다음으로 넘김.
							_LOG(TEXT("Effect item [%d]'s parent [%d] is not processed yet; Idx = %d, NP = %d"), _id, _idParent, Idx, NPList.Num());
							continue;
						}
					}

					FavaEffectItemDesc *_newDesc = new FavaEffectItemDesc(_id, _parent, _sales_id);
					check(_newDesc);
					effectItemList.Set(_id, _newDesc);
					effectItemArray.Push(_newDesc);

					_desc = _newDesc;
				}

				if (!_desc)
				{
					_LOG(TEXT("Error! Unknown item category; section [%s]; category = %d"), _ini.GetCurrentSectionName(), HIBYTE(_id));
					NPList.Remove(Idx);
					break;
				}

				// BASE_ITEM_DESC 채우기
				_READ_VAL_INT(_desc, code, TID_ITEM, TEXT("Item_Code"));
				if (_desc->code == ID_INVALID_ITEM)
					_desc->code = _id;

				_READ_VAL_INT(_desc, useLimitLevel, BYTE, TEXT("UseLimit_LV"));
				_READ_VAL_INT(_desc, gaugeType, BYTE, TEXT("GaugeType"));
				_READ_VAL_INT(_desc, Durability_Game_Drop, DWORD, TEXT("Durability_Game_Drop"));
				_READ_VAL_INT(_desc, Durability_Time_Drop, DWORD, TEXT("Durability_Time_Drop"));
				_READ_VAL_INT(_desc, maintenancePrice, TMONEY, TEXT("Maintenance_Price"));
				_READ_VAL_INT(_desc, dateLimit, BYTE, TEXT("DateLimit"));

				_READ_VAL_INT(_desc, priceType, BYTE, TEXT("PriceType"));
				_READ_VAL_INT(_desc, price, TMONEY, TEXT("Price"));

				switch (_desc->GetItemFlag())
				{
				case _IF_WEAPON:
				case _IF_EQUIP:
					{
						// 무기/군장
						FavaItemDesc *_itemDesc = static_cast<FavaItemDesc*>(_desc);

						_READ_VAL_INT(_itemDesc, destroyable, BYTE, TEXT("IsDestroyable"));
						_READ_VAL_INT(_itemDesc, statLog, BYTE, TEXT("bStatLog"));
						_READ_VAL_INT(_itemDesc, isDefaultItem, BYTE, TEXT("IsDefaultItem"));

						_READ_VAL_INT(_itemDesc, bonusMoney, TMONEY, TEXT("BonusMoney"));

						if (_ini.GetValue(TEXT("EquipSetType"), szTemp))
						{
							_itemDesc->slotType = 0;
							TCHAR* _tok	= _tcstok(szTemp,TEXT(","));
							while(_tok != NULL)
							{
								DWORD _slotType	= EquipStrToValue(_tok);
								if(_slotType !=	_EP_NONE)
								{
									_itemDesc->slotType |= _slotType;
								}
								else
									break;
								_tok = _tcstok(	NULL, TEXT(",")	);
							}
						}

						_READ_VAL_INT(_itemDesc, bRisConvertible, BYTE, TEXT("bRISConvertible"));
						_READ_VAL_INT(_itemDesc, RisConvertibleID, TID_ITEM, TEXT("bRISConvertibleID"));
						_READ_VAL_INT(_itemDesc, RisConvertiblePrice, TMONEY, TEXT("bRISConvertiblePrice"));

						// custom slot type	load..
						if (IsWeaponItem(_itemDesc->id))
						{
							if (_ini.GetValue(TEXT("CustomSlot_Type"), szTemp))
							{
								_itemDesc->customType.type = 0;
								TCHAR* _tok	= _tcstok(szTemp,TEXT(","));
								while(_tok != NULL)
								{
									switch(CustomStrToValue(_tok))
									{
									case _CSI_FRONT:
										_itemDesc->customType.front = 1;
										break;
									case _CSI_MOUNT:
										_itemDesc->customType.mount = 1;
										break;
									case _CSI_BARREL:
										_itemDesc->customType.barrel = 1;
										break;
									case _CSI_TRIGGER:
										_itemDesc->customType.trigger = 1;
										break;
									case _CSI_GRIP:
										_itemDesc->customType.grip = 1;
										break;
									case _CSI_STOCK:
										_itemDesc->customType.stock = 1;
										break;
									}
									_tok = _tcstok(	NULL, TEXT(",")	);
								}
							}
							if (_itemDesc->customType.mount)
								_READ_VAL_INT(_itemDesc, defaultCustomItem, TID_ITEM, TEXT("CustomSlot_Default_M"));

						}

						_READ_VAL_FUNC(_itemDesc, effectInfo.effectType, EffectStrToValue, TEXT("Item_Effect"));
						if (_itemDesc->effectInfo.effectType > 0)
						{
							_READ_VAL_INT(_itemDesc, effectInfo.effectValue, int, TEXT("Item_EffectValue"));
						}

						_itemDesc->ItemName = _ini.GetString(TEXT("Item_Name"), TEXT(""));
						_itemDesc->Description = _ini.GetString(TEXT("Item_Description"), TEXT(""));
						_itemDesc->IconChar = (*_ini.GetString(TEXT("Item2DFont"), TEXT("a")))[0];

						if (_ini.GetValue(TEXT("Item_GRAPH_Value"), szTemp))
						{
							int	_idx = 0;
							TCHAR* _tok	= _tcstok(szTemp,TEXT(","));
							while(_tok != NULL && _idx < MAX_ITEM_GRAPH_LIST)
							{
								int	_value = _ttoi(_tok);
								_itemDesc->GraphValue[_idx++] = (short)_value;
								_tok = _tcstok(	NULL, TEXT(",")	);
							}
						}
					}
					break;
				case _IF_CUSTOM:
					{
						FavaCustomItemDesc *_itemDesc = static_cast<FavaCustomItemDesc*>(_desc);

						_READ_VAL_INT(_itemDesc, item_id, TID_ITEM, TEXT("CustomWeapon_ID"));
						_READ_VAL_FUNC(_itemDesc, customType, CustomStrToValue, TEXT("EquipSetType"));

						_READ_VAL_INT(_itemDesc, isDefaultItem, BYTE, TEXT("IsDefaultItem"));

						_itemDesc->ItemName = _ini.GetString(TEXT("Item_Name"), TEXT(""));
						_itemDesc->Description = _ini.GetString(TEXT("Item_Description"), TEXT(""));
						_itemDesc->IconChar = (*_ini.GetString(TEXT("Item2DFont"), TEXT("a")))[0];

						if (_ini.GetValue(TEXT("Item_GRAPH_Value"), szTemp))
						{
							int	_idx = 0;
							TCHAR* _tok	= _tcstok(szTemp,TEXT(","));
							while(_tok != NULL && _idx < MAX_ITEM_GRAPH_LIST)
							{
								int	_value = _ttoi(_tok);
								_itemDesc->GraphValue[_idx++] = (short)_value;
								_tok = _tcstok(	NULL, TEXT(",")	);
							}
						}

						_CHECK_COND(_itemDesc->gaugeType != _IGT_MAINTENANCE, gaugeType);
						_CHECK_COND(_itemDesc->priceType != _IPT_CASH, priceType);
						//_CHECK_COND(_itemDesc->item_id != ID_INVALID_ITEM, item_id);
						//_CHECK_COND(_itemDesc->customType > _CSI_NONE && _itemDesc->customType < _CSI_MAX, customType);
					}
					break;
				case _IF_EFFECT:
					{
						FavaEffectItemDesc *_itemDesc = static_cast<FavaEffectItemDesc*>(_desc);

						_READ_VAL_INT(_itemDesc, bonusMoney, TMONEY, TEXT("BonusMoney"));

						if (_ini.GetValue(TEXT("EquipSetType"), szTemp))
						{
							_itemDesc->slotType = 0;
							TCHAR* _tok	= _tcstok(szTemp,TEXT(","));
							while(_tok != NULL)
							{
								DWORD _slotType	= EquipStrToValue(_tok);
								if(_slotType !=	_EP_NONE)
								{
									_itemDesc->slotType |= _slotType;
								}
								else
									break;
								_tok = _tcstok(	NULL, TEXT(",")	);
							}
						}

						_READ_VAL_FUNC(_itemDesc, effectInfo.effectType, EffectStrToValue, TEXT("Item_Effect"));
						if (_itemDesc->effectInfo.effectType > 0)
						{
							_READ_VAL_INT(_itemDesc, effectInfo.effectValue, int, TEXT("Item_EffectValue"));
						}

						_itemDesc->ItemName = _ini.GetString(TEXT("Item_Name"), TEXT(""));
						_itemDesc->Description = _ini.GetString(TEXT("Item_Description"), TEXT(""));
						_itemDesc->IconChar = (*_ini.GetString(TEXT("Item2DFont"), TEXT("a")))[0];

						_CHECK_COND(_itemDesc->gaugeType != _IGT_MAINTENANCE, gaugeType);
					}
					break;
				default:
					break;
				}

				NPList.Remove(Idx);
			}
			BREAK_SECTION_END()

			++Idx;
			if (!_ini.SelectNextSection())
			{
				// ini를 한 번 다 처리했음
				if (NPList.Num() > 0 && NPList.Num() == PrevNPListNum)
				{
					// 처리가 불가능한 섹션이 존재함
					_LOG(TEXT("Error! Item with invalid parent id exists! NPList.Num() = %d"), NPList.Num());
					TMap<INT,INT>::TIterator it(NPList);
					for (INT i = 0; it; ++i, ++it)
					{
						_LOG(TEXT("[%d]%d"), i, it.Value());
					}
					return FALSE;
				}
				Idx = 0;
				_ini.SelectFirstSection();
				PrevNPListNum = NPList.Num();
			}
		}	// while (NPList.Num() > 0)

		_LOG(TEXT("%d faces, %d items, %d custom items, %d effect items loaded from %s"),
				faceList.Num(), itemList.Num(), customItemList.Num(), effectItemList.Num(), *ItemDescName);

		//DumpFace();
		//DumpWeapon();
		//DumpEquip();
		//DumpCustom();
		//DumpEffect();
	}

	// SLOT	DESC Load...
	{
		FString	SlotDescName;
		if ( !GConfig->GetString(CFG_SECTION, CFG_SLOTDESC,	SlotDescName, GNetIni) || SlotDescName == TEXT("") )
			SlotDescName = TEXT("SlotDesc.ini");
		
		CIni _ini(*SlotDescName);

		UBOOL Res = (bEncrypted ? _ini.OpenConfigAES() : _ini.OpenConfig());
		if (Res)
			Res = _ini.SelectFirstSection();

		if(!Res)
		{
			_LOG(TEXT("Error! Failed to load slot list; file = %s"), *SlotDescName);
			return FALSE;
		}

		TCHAR _section[32];
		SLOT_DESC _slotDesc;

		for(INT	_i=0;_i<MAX_EQUIPSET_SIZE;++_i)
		{
			_stprintf(_section,TEXT("EQUIP_%d"),_i);
			if(!_ini.SelectSection(_section))
			{
				_LOG(TEXT("Error! Failed to read section [%s]"), _section);
				return FALSE;
			}

			appMemzero(&_slotDesc,sizeof(SLOT_DESC));

			_slotDesc.index	= (TID_EQUIP_SLOT)_ini.GetInteger(TEXT("index"),ID_INVALID_EQUIP_SLOT);
			if(_slotDesc.index <0 || _slotDesc.index >=	MAX_EQUIPSET_SIZE)
			{
				_LOG(TEXT("Error! Invalid slot index(%d) from section [%s]"), _slotDesc.index, _section);
				return FALSE;
			}

			if(_ini.GetValue(TEXT("EquipSetType"),szTemp))
			{
				_slotDesc.slotType = EquipStrToValue(szTemp);
				if(_slotDesc.slotType == _EP_NONE)
				{
					_LOG(TEXT("Error! Invalid slot type(%s) from section [%s]"), szTemp, _section);
					return FALSE;
				}
			}

			_slotDesc.defaultItem =	(TID_ITEM)_ini.GetInteger(TEXT("DefaultItem"),ID_INVALID_ITEM);
			ITEM_DESC *pItem = GetItem(_slotDesc.defaultItem);
			if (pItem && !(pItem->slotType & _slotDesc.slotType))
			{
				_LOG(TEXT("Error! Failed to get default item(%d) description of section [%s]; pItem->slotType = %d, _slotDesc.slotType = %d"),
							_slotDesc.defaultItem, _section, (pItem ? pItem->slotType : -1), _slotDesc.slotType);
				return FALSE;
			}

			equipSlotDesc[_slotDesc.index] = _slotDesc;

			//_LOG(TEXT("Slot: index = %d, defaultItem = %d, slotType = %s(%d)"), _slotDesc.index, _slotDesc.defaultItem, szTemp, _slotDesc.slotType);
		}

		for(INT	_i=0;_i<MAX_WEAPONSET_SIZE;++_i)
		{
			_stprintf(_section,TEXT("WEAPON_%d"),_i);
			if(!_ini.SelectSection(_section))
			{
				_LOG(TEXT("Error! Failed to read section [%s]"), _section);
				return FALSE;
			}

			appMemzero(&_slotDesc, sizeof(SLOT_DESC));

			_slotDesc.index	= (TID_EQUIP_SLOT)_ini.GetInteger(TEXT("index"),ID_INVALID_EQUIP_SLOT);
			if(_slotDesc.index < 0 || _slotDesc.index >=	MAX_WEAPONSET_SIZE)
			{
				_LOG(TEXT("Error! Invalid slot index(%d) from section [%s]"), _slotDesc.index, _section);
				return FALSE;
			}

			if(_ini.GetValue(TEXT("EquipSetType"),szTemp))
			{
				_slotDesc.slotType = EquipStrToValue(szTemp);
				if(_slotDesc.slotType == _EP_NONE)
				{
					_LOG(TEXT("Error! Invalid slot type(%s) from section [%s]"), szTemp, _section);
					return FALSE;
				}
			}

			_slotDesc.defaultItem =	(TID_ITEM)_ini.GetInteger(TEXT("DefaultItem"),ID_INVALID_ITEM);
			ITEM_DESC *pItem = GetItem(_slotDesc.defaultItem);
			if (pItem && !(pItem->slotType & _slotDesc.slotType))
			{
				_LOG(TEXT("Error! Failed to get default item(%d) description of section [%s]; pItem->id = %d, pItem->itemName = %s, pItem->slotType = %d, _slotDesc.slotType = %d"),
							_slotDesc.defaultItem, _section, pItem->id, pItem->GetName(), pItem->slotType, _slotDesc.slotType);
				return FALSE;
			}

			weaponSlotDesc[_slotDesc.index]	= _slotDesc;

			//_LOG(TEXT("Slot: index = %d, defaultItem = %d, slotType = %s(%d)"), _slotDesc.index, _slotDesc.defaultItem, szTemp, _slotDesc.slotType);
		}

		for(INT	_i=0;_i<MAX_EFFECTSET_SIZE;++_i)
		{
			_stprintf(_section,TEXT("EFFECT_%d"),_i);
			if(!_ini.SelectSection(_section))
			{
				_LOG(TEXT("Error! Failed to read section [%s]"), _section);
				return FALSE;
			}

			appMemzero(&_slotDesc, sizeof(SLOT_DESC));

			_slotDesc.index	= (TID_EQUIP_SLOT)_ini.GetInteger(TEXT("index"),ID_INVALID_EQUIP_SLOT);
			if(_slotDesc.index <0 || _slotDesc.index >=	MAX_EFFECTSET_SIZE)
			{
				_LOG(TEXT("Error! Invalid slot index(%d) from section [%s]"), _slotDesc.index, _section);
				return FALSE;
			}

			if(_ini.GetValue(TEXT("EquipSetType"),szTemp))
			{
				_slotDesc.slotType = EquipStrToValue(szTemp);
				if(_slotDesc.slotType == _EP_NONE)
				{
					_LOG(TEXT("Error! Invalid slot type(%s) from section [%s]"), szTemp, _section);
					return FALSE;
				}
			}

			_slotDesc.defaultItem =	(TID_ITEM)_ini.GetInteger(TEXT("DefaultItem"),ID_INVALID_ITEM);
			EFFECT_ITEM_DESC *pItem = GetEffectItem(_slotDesc.defaultItem);
			if (pItem && !(pItem->slotType & _slotDesc.slotType))
			{
				_LOG(TEXT("Error! Failed to get default item(%d) description of section [%s]; pItem->slotType = %d, _slotDesc.slotType = %d"),
							_slotDesc.defaultItem, _section, (pItem ? pItem->slotType : -1), _slotDesc.slotType);
				return FALSE;
			}

			effectSlotDesc[_slotDesc.index] = _slotDesc;

			//_LOG(TEXT("Slot: index = %d, defaultItem = %d, slotType = %s(%d)"), _slotDesc.index, _slotDesc.defaultItem, szTemp, _slotDesc.slotType);
		}

		_LOG(TEXT("slot descriptions loaded from %s"), *SlotDescName);
	}

	bInit = TRUE;

	return TRUE;
}

void CavaItemDescData::Final()
{
	if (bInit)
	{
		for (INT i = 0; i < itemArray.Num(); ++i)
		{
			delete itemArray(i);
		}
		itemArray.Empty();
		itemList.Empty();

		for (INT i = 0; i < customItemArray.Num(); ++i)
		{
			delete customItemArray(i);
		}
		customItemArray.Empty();
		customItemList.Empty();

		for (INT i = 0; i < effectItemArray.Num(); ++i)
		{
			delete effectItemArray(i);
		}
		effectItemArray.Empty();
		effectItemList.Empty();

		bInit = FALSE;

//		_LOG(TEXT("All items unloaded."));
	}
}

ITEM_DESC* CavaItemDescData::GetItem(TID_ITEM idItem)
{
	return itemList.FindRef(idItem);
}

ITEM_DESC* CavaItemDescData::GetItemByIndex(int index)
{
	return itemArray.IsValidIndex(index) ? itemArray(index) : NULL;
}

CUSTOM_ITEM_DESC* CavaItemDescData::GetCustomItem(TID_ITEM idItem)
{
	return customItemList.FindRef(idItem);
}

CUSTOM_ITEM_DESC* CavaItemDescData::GetCustomItemByIndex(int index)
{
	return customItemArray.IsValidIndex(index) ? customItemArray(index) : NULL;
}

EFFECT_ITEM_DESC* CavaItemDescData::GetEffectItem(TID_ITEM idItem)
{
	return effectItemList.FindRef(idItem);
}

EFFECT_ITEM_DESC* CavaItemDescData::GetEffectItemByIndex(int index)
{
	return effectItemArray.IsValidIndex(index) ? effectItemArray(index) : NULL;
}

BOOL CavaItemDescData::FaceExists(TID_ITEM idFace)
{
	return faceList.ContainsItem(idFace);
}

SLOT_DESC* CavaItemDescData::GetWeaponSlot(TID_EQUIP_SLOT slot)
{
	if(slot <0 || slot >= MAX_WEAPONSET_SIZE)
		return NULL;

	for(INT _i=0;_i<MAX_WEAPONSET_SIZE;++_i)
	{
		if(weaponSlotDesc[_i].index == slot)
		{
			return &weaponSlotDesc[_i];
		}
	}
	return NULL;
}

SLOT_DESC* CavaItemDescData::GetWeaponSlotByType(int slot_type)
{
	for (INT i = 0; i < MAX_WEAPONSET_SIZE; ++i)
	{
		if (slot_type & weaponSlotDesc[i].slotType)
		{
			return &weaponSlotDesc[i];
		}
	}

	return NULL;
}

SLOT_DESC* CavaItemDescData::GetEquipSlot(TID_EQUIP_SLOT slot)
{
	if (slot < 0 || slot >= MAX_EQUIPSET_SIZE)
		return NULL;

	for (INT _i = 0; _i < MAX_EQUIPSET_SIZE; ++_i)
	{
		if (equipSlotDesc[_i].index == slot)
		{
			return &equipSlotDesc[_i];
		}
	}
	return NULL;
}

SLOT_DESC* CavaItemDescData::GetEquipSlotByType(int slot_type)
{
	for (INT i = 0; i < MAX_EQUIPSET_SIZE; ++i)
	{
		if (slot_type & equipSlotDesc[i].slotType)
		{
			return &equipSlotDesc[i];
		}
	}
	return NULL;
}

SLOT_DESC* CavaItemDescData::GetEffectSlot(TID_EQUIP_SLOT slot)
{
	if (slot < 0 || slot >= MAX_EFFECTSET_SIZE)
		return NULL;

	for (INT _i = 0; _i < MAX_EFFECTSET_SIZE; ++_i)
	{
		if (effectSlotDesc[_i].index == slot)
		{
			return &effectSlotDesc[_i];
		}
	}
	return NULL;
}

SLOT_DESC* CavaItemDescData::GetEffectSlotByType(int slot_type)
{
	for (INT i = 0; i < MAX_EFFECTSET_SIZE; ++i)
	{
		if (slot_type & effectSlotDesc[i].slotType)
		{
			return &effectSlotDesc[i];
		}
	}
	return NULL;
}


INT CavaItemDescData::GetAvailableWeaponsBySlot(INT idSlot, TArray<INT>& ItemList)
{
	ItemList.Empty();

	{
		INT Mask = (1 << idSlot);
		for (TMAP_ITEMLIST::TIterator it(itemList); it; ++it)
		{
			if (IsWeaponItem(it.Value()->id) && it.Value()->slotType & Mask)
			{
				ItemList.Push(it.Value()->id);
			}
		}
	}

	return ItemList.Num();
}

INT CavaItemDescData::GetAvailableEquipsBySlot(INT idSlot, TArray<INT>& ItemList)
{
	ItemList.Empty();

	if (idSlot == 22)
	{
		for (INT i = 0; i < faceList.Num(); ++i)
			ItemList.Push(faceList(i));

	}
	else
	{
		INT Mask = (1 << idSlot);
		for (TMAP_ITEMLIST::TIterator it(itemList); it; ++it)
		{
			if (IsEquipItem(it.Value()->id) && it.Value()->slotType & Mask)
			{
				ItemList.Push(it.Value()->id);
			}
		}
	}

	return ItemList.Num();
}

INT CavaItemDescData::GetAvailableEffectsBySlot(INT idSlot, TArray<INT>& ItemList)
{
	ItemList.Empty();

	{
		INT Mask = (1 << idSlot);
		for (TMAP_EFFECTITEMLIST::TIterator it(effectItemList); it; ++it)
		{
			if (it.Value()->slotType & Mask)
			{
				ItemList.Push(it.Value()->id);
			}
		}
	}

	return ItemList.Num();
}

void CavaItemDescData::DumpFace()
{
#if !FINAL_RELEASE
	for (INT i = 0; i < faceList.Num(); ++i)
	{
		_LOG(TEXT("Face[%d] id = %d"), i, faceList(i));
	}
	_LOG(TEXT("%d faces loaded."), faceList.Num());
#endif
}

void CavaItemDescData::DumpWeapon()
{
#if !FINAL_RELEASE
	INT Cnt = 0;
	for (INT i = 0; i < itemArray.Num(); ++i)
	{
		BASE_ITEM_DESC *Item = itemArray(i);
		check(Item);
		if (IsWeaponItem(Item->id))
		{
			_LOG(TEXT("Weapon[%d] id = %d, parent = %d, name = %s, %s = %d; %s"),
				Cnt, Item->id, Item->idParent, Item->GetName(),
				(Item->priceType == _IPT_MONEY ? TEXT("euro") : Item->priceType == _IPT_CASH ? TEXT("cash") : TEXT("price")), Item->price, Item->GetDescription());
			++Cnt;
		}
	}
	_LOG(TEXT("%d weapons loaded."), Cnt);
#endif
}

void CavaItemDescData::DumpEquip()
{
#if !FINAL_RELEASE
	INT Cnt = 0;
	for (INT i = 0; i < itemArray.Num(); ++i)
	{
		BASE_ITEM_DESC *Item = itemArray(i);
		check(Item);
		if (IsEquipItem(Item->id))
		{
			_LOG(TEXT("Equip[%d] id = %d, parent = %d, name = %s, %s = %d; %s"),
				Cnt, Item->id, Item->idParent, Item->GetName(),
				(Item->priceType == _IPT_MONEY ? TEXT("euro") : Item->priceType == _IPT_CASH ? TEXT("cash") : TEXT("price")), Item->price, Item->GetDescription());
			++Cnt;
		}
	}
	_LOG(TEXT("%d equips loaded."), Cnt);
#endif
}

void CavaItemDescData::DumpCustom()
{
#if !FINAL_RELEASE
	INT Cnt = 0;
	for (INT i = 0; i < customItemArray.Num(); ++i)
	{
		BASE_ITEM_DESC *Item = customItemArray(i);
		check(Item);
		{
			_LOG(TEXT("Custom[%d] id = %d, parent = %d, name = %s, %s = %d; %s"),
				Cnt, Item->id, Item->idParent, Item->GetName(),
				(Item->priceType == _IPT_MONEY ? TEXT("euro") : Item->priceType == _IPT_CASH ? TEXT("cash") : TEXT("price")), Item->price, Item->GetDescription());
			++Cnt;
		}
	}
	_LOG(TEXT("%d customs loaded."), Cnt);
#endif
}

void CavaItemDescData::DumpEffect()
{
#if !FINAL_RELEASE
	INT Cnt = 0;
	for (INT i = 0; i < effectItemArray.Num(); ++i)
	{
		BASE_ITEM_DESC *Item = effectItemArray(i);
		check(Item);
		{
			_LOG(TEXT("Effect[%d] id = %d, parent = %d, name = %s, %s = %d"),
				Cnt, Item->id, Item->idParent, Item->GetName(),
				(Item->priceType == _IPT_MONEY ? TEXT("euro") : Item->priceType == _IPT_CASH ? TEXT("cash") : TEXT("price")), Item->price);
			++Cnt;
		}
	}
	_LOG(TEXT("%d effects loaded."), Cnt);
#endif
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ShopDesc


CavaShopDesc::CavaShopDesc() : bInit(FALSE)
{
	if ( !Init() )
	{
		//__asm { int 3 }
	}

}

CavaShopDesc::~CavaShopDesc()
{
	//if (bInit)
	//{
	//}
}


BOOL CavaShopDesc::Init()
{
	if (bInit)
		return FALSE;

	const BOOL bEncrypted =	TRUE;
	{
		FString	ShopDescName;
		if ( !GConfig->GetString(CFG_SECTION, CFG_SHOPDESC, ShopDescName, GNetIni) || ShopDescName == TEXT("") )
			ShopDescName = TEXT("ShopDesc.ini");

		CIni _ini(*ShopDescName);

		UBOOL Res = (bEncrypted ? _ini.OpenConfigAES() : _ini.OpenConfig());
		if (Res)
			Res = _ini.SelectFirstSection();

		if(!Res)
		{
			_LOG(TEXT("Error! Failed to load shop item list; file = %s"), *ShopDescName);
			return FALSE;
		}

		do
		{
			TID_SHOPITEM _id = (TID_SHOPITEM)_ini.GetInteger(TEXT("ShopItem_ID"),ID_INVALID_SHOPITEM);
			if (_id	== ID_INVALID_SHOPITEM)
			{
				_LOG(TEXT("Error! Failed to read ShopItem_ID from section [%s]"), _ini.GetCurrentSectionName());
				return FALSE;
			}

			FavaShopItem *ShopItem = new FavaShopItem(_id);
			check(ShopItem);

			ShopItem->ShopItemName = _ini.GetString(TEXT("ShopItem_Name"), TEXT(""));
			if (ShopItem->ShopItemName.Len() == 0)
			{
				_LOG(TEXT("Error! Failed to read ShopItem_Name from section [%s]"), _ini.GetCurrentSectionName());
				delete ShopItem;
				return FALSE;
			}

			FString IDListStr = _ini.GetString(TEXT("Item_ID"), TEXT(""));
			FString NameListStr = _ini.GetString(TEXT("ShopOption_Name"), TEXT(""));
			TArray<FString> IDList, NameList;

			IDListStr.ParseIntoArray(&IDList, TEXT(","), FALSE);
			NameListStr.ParseIntoArray(&NameList, TEXT(","), FALSE);
			if (IDList.Num() != NameList.Num())
			{
				_LOG(TEXT("Warning! Number of ShopOption_Name and Item_ID is different; Section [%s]"), _ini.GetCurrentSectionName());
				//delete ShopItem;
				//return FALSE;
			}
			INT ItemFlag = _IF_NONE;
			for (INT i = 0; i < IDList.Num(); ++i)
			{
				FavaShopItem::FavaShopOption Option((TID_ITEM)appAtoi(*IDList(i)), NameList.IsValidIndex(i) ? NameList(i) : TEXT(""));
				if (Option.OptionID == ID_INVALID_ITEM)
				{
					_LOG(TEXT("Error! One of Item_ID is invalid; Section [%s]"), _ini.GetCurrentSectionName());
					return FALSE;
				}
				if (ItemFlag == _IF_NONE)
				{
					if (IsWeaponItem(Option.OptionID))
						ItemFlag = _IF_WEAPON;
					else if (IsEquipItem(Option.OptionID))
						ItemFlag = _IF_EQUIP;
					else if (IsCustomItem(Option.OptionID))
						ItemFlag = _IF_CUSTOM;
					else if (IsEffectItem(Option.OptionID))
						ItemFlag = _IF_EFFECT;
					else
					{
						_LOG(TEXT("Error! Unknown item category; Section [%s], OptionID = %d, Category = %d"),
								_ini.GetCurrentSectionName(), Option.OptionID, HIBYTE(Option.OptionID));
						continue;
					}
				}
				else if ( (ItemFlag == _IF_WEAPON && !IsWeaponItem(Option.OptionID)) ||
						(ItemFlag == _IF_EQUIP && !IsEquipItem(Option.OptionID)) ||
						(ItemFlag == _IF_CUSTOM && !IsCustomItem(Option.OptionID)) ||
						(ItemFlag == _IF_EFFECT && !IsEffectItem(Option.OptionID)) )
				{
					_LOG(TEXT("Error! Options have different item categories each other; Section [%s]"), _ini.GetCurrentSectionName());
					return FALSE;
				}

				switch (ItemFlag)
				{
				case _IF_WEAPON:
				case _IF_EQUIP:
					Option.pItem = _ItemDesc().GetItem(Option.OptionID);
					break;
				case _IF_CUSTOM:
					Option.pItem = _ItemDesc().GetCustomItem(Option.OptionID);
					break;
				case _IF_EFFECT:
					Option.pItem = _ItemDesc().GetEffectItem(Option.OptionID);
					break;
				}

				if (!Option.pItem)
				{
					_LOG(TEXT("Error! Unknown item; Section [%s], Item_ID [%d]"), _ini.GetCurrentSectionName(), Option.OptionID);
					continue;
					//return FALSE;
				}

				//_LOG(TEXT("Option.pItem->id = %d, Option.pItem->itemName = %s; %s"), Option.pItem->id, Option.pItem->GetName(), Option.pItem->GetDescription());

				ShopItem->Options.Push(Option);
			}
			if (ShopItem->Options.Num() == 0 || ItemFlag == _IF_NONE)
			{
				_LOG(TEXT("Error! No options found in section [%s]"), _ini.GetCurrentSectionName());
				delete ShopItem;
				continue;
			}

			ShopItem->DisplayType = _ini.GetInteger(TEXT("DisplayType"), _IDT_NONE);
			ShopItem->DisplayPriority = _ini.GetInteger(TEXT("DisplayPriority"), 0);
			ShopItem->Icon = _ini.GetString(TEXT("Item2DFont"), TEXT(" "))[0];
			ShopItem->Description = _ini.GetString(TEXT("Item_Description"), TEXT(""));

			switch (ItemFlag)
			{
			case _IF_WEAPON:
			case _IF_EQUIP:
				{
					shopItemList.Set(ShopItem->ShopItemID, ShopItem);
					shopItemArray.Push(ShopItem);
				}
				break;
			case _IF_CUSTOM:
				{
					shopCustomItemList.Set(ShopItem->ShopItemID, ShopItem);
					shopCustomItemArray.Push(ShopItem);
				}
				break;
			case _IF_EFFECT:
				{
					shopEffectItemList.Set(ShopItem->ShopItemID, ShopItem);
					shopEffectItemArray.Push(ShopItem);
				}
				break;
			}
		}
		while (_ini.SelectNextSection());

		_LOG(TEXT("%d items, %d custom items, %d effect items loaded from %s"),
				shopItemList.Num(), shopCustomItemList.Num(), shopEffectItemList.Num(), *ShopDescName);

		//DumpWeapon();
		//DumpEquip();
		//DumpCustom();
		//DumpEffect();
	}

	bInit = TRUE;

	return TRUE;
}

void CavaShopDesc::Final()
{
	if (bInit)
	{
		for (INT i = 0; i < shopItemArray.Num(); ++i)
		{
			delete shopItemArray(i);
		}
		shopItemArray.Empty();
		shopItemList.Empty();

		for (INT i = 0; i < shopCustomItemArray.Num(); ++i)
		{
			delete shopCustomItemArray(i);
		}
		shopCustomItemArray.Empty();
		shopCustomItemList.Empty();

		for (INT i = 0; i < shopEffectItemArray.Num(); ++i)
		{
			delete shopEffectItemArray(i);
		}
		shopEffectItemArray.Empty();
		shopEffectItemList.Empty();

		//_LOG(TEXT("All items unloaded."));
	}
}

FavaShopItem* CavaShopDesc::GetItem(TID_SHOPITEM idItem)
{
	return shopItemList.FindRef(idItem);
}

FavaShopItem* CavaShopDesc::GetItemByIndex(INT index)
{
	return shopItemArray.IsValidIndex(index) ? shopItemArray(index) : NULL;
}

FavaShopItem* CavaShopDesc::GetCustomItem(TID_SHOPITEM idItem)
{
	return shopCustomItemList.FindRef(idItem);
}

FavaShopItem* CavaShopDesc::GetCustomItemByIndex(INT index)
{
	return shopCustomItemArray.IsValidIndex(index) ? shopCustomItemArray(index) : NULL;
}

FavaShopItem* CavaShopDesc::GetEffectItem(TID_SHOPITEM idItem)
{
	return shopEffectItemList.FindRef(idItem);
}

FavaShopItem* CavaShopDesc::GetEffectItemByIndex(INT index)
{
	return shopEffectItemArray.IsValidIndex(index) ? shopEffectItemArray(index) : NULL;
}

void CavaShopDesc::DumpWeapon()
{
#if !FINAL_RELEASE
	INT Cnt = 0;
	for (INT i = 0; i < shopItemArray.Num(); ++i)
	{
		FavaShopItem *Item = shopItemArray(i);
		check(Item);
		if (Item->GetItemFlag() == _IF_WEAPON)
		{
			_LOG(TEXT("ShopWeapon[%d] id = %d, name = %s, options = %d; %s"),
				Cnt, Item->ShopItemID, *Item->GetName(), Item->Options.Num(), *Item->GetDescription());
			++Cnt;
		}
	}
	_LOG(TEXT("%d weapons loaded."), Cnt);
#endif
}

void CavaShopDesc::DumpEquip()
{
#if !FINAL_RELEASE
	INT Cnt = 0;
	for (INT i = 0; i < shopItemArray.Num(); ++i)
	{
		FavaShopItem *Item = shopItemArray(i);
		check(Item);
		if (Item->GetItemFlag() == _IF_EQUIP)
		{
			_LOG(TEXT("ShopEquip[%d] id = %d, name = %s, options = %d; %s"),
				Cnt, Item->ShopItemID, *Item->GetName(), Item->Options.Num(), *Item->GetDescription());
			++Cnt;
		}
	}
	_LOG(TEXT("%d equips loaded."), Cnt);
#endif
}

void CavaShopDesc::DumpCustom()
{
#if !FINAL_RELEASE
	INT Cnt = 0;
	for (INT i = 0; i < shopCustomItemArray.Num(); ++i)
	{
		FavaShopItem *Item = shopCustomItemArray(i);
		check(Item);
		//if (Item->GetItemFlag() == _IF_WEAPON)
		{
			_LOG(TEXT("ShopCustom[%d] id = %d, name = %s, options = %d; %s"),
				Cnt, Item->ShopItemID, *Item->GetName(), Item->Options.Num(), *Item->GetDescription());
			++Cnt;
		}
	}
	_LOG(TEXT("%d customs loaded."), Cnt);
#endif
}

void CavaShopDesc::DumpEffect()
{
#if !FINAL_RELEASE
	INT Cnt = 0;
	for (INT i = 0; i < shopEffectItemArray.Num(); ++i)
	{
		FavaShopItem *Item = shopEffectItemArray(i);
		check(Item);
		//if (Item->GetItemFlag() == _IF_WEAPON)
		{
			_LOG(TEXT("ShopEffect[%d] id = %d, name = %s, options = %d; %s"),
				Cnt, Item->ShopItemID, *Item->GetName(), Item->Options.Num(), *Item->GetDescription());
			++Cnt;
		}
	}
	_LOG(TEXT("%d effects loaded."), Cnt);
#endif
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WordCensor


CavaWordFilter::CavaWordFilter(TCHAR *path)
{
	if (path)
	{
		FilePath = path;
		FilePath = FilePath.ToUpper();
	}
	else
		FilePath = TEXT("");
}

CavaWordFilter::~CavaWordFilter()
{
}

BOOL CavaWordFilter::Init()
{
	const BOOL bEncrypted = TRUE;

	FString	_Path;
	if (FilePath == TEXT("W1.TXT"))
		GConfig->GetString(CFG_SECTION, CFG_W1, _Path, GNetIni);
	else if (FilePath == TEXT("W2.TXT"))
		GConfig->GetString(CFG_SECTION, CFG_W2, _Path, GNetIni);

	if (_Path == TEXT(""))
	{
		_Path = FilePath;
	}

	TArray<FString> Lines;

	if ( LoadStringsFromFile(Lines, (TCHAR*)*_Path, bEncrypted) == 0 )
	{
		_LOG(TEXT("Error! Failed to load word list; file = %s"), *_Path);
		return FALSE;
	}

	for (INT i = 0; i < Lines.Num(); ++i)
		SetCensorWord((TCHAR*)*Lines(i));

	_LOG(TEXT("%d words loaded from %s"), Lines.Num(), *_Path);

	///////////////////////////////////////////////////////////////////

	if (FilePath == TEXT("W1.TXT"))
	{
		FString IgnoreCh;

		if ( GConfig->GetString(CFG_SECTION, CFG_W1_IGNORE, IgnoreCh, GNetIni) && IgnoreCh != TEXT("") )
		{
			TCHAR *Ch = (TCHAR*)*IgnoreCh;
			while (*Ch)
			{
				SetIgnoreCh(*Ch);
				++Ch;
			}

			_LOG(TEXT("Ignore characters loaded [%s]"), *IgnoreCh);
		}
	}
	else if (FilePath == TEXT("W2.TXT"))
	{
		FString IgnoreCh;

		if ( GConfig->GetString(CFG_SECTION, CFG_W2_IGNORE, IgnoreCh, GNetIni) && IgnoreCh != TEXT("") )
		{
			TCHAR *Ch = (TCHAR*)*IgnoreCh;
			while (*Ch)
			{
				SetIgnoreCh(*Ch);
				++Ch;
			}

			_LOG(TEXT("Ignore characters loaded [%s]"), *IgnoreCh);
		}
	}

	return TRUE;
}

void CavaWordFilter::SetIgnoreCh(TCHAR ch)
{
	ch = _totupper(ch);
	if ( !setIgnoreCh.HasKey(ch) )
		setIgnoreCh.Set(ch, 0);
}

void CavaWordFilter::ReplaceIgnoreCh(TCHAR *str)
{
	INT _len = _tcslen(str);
	for(INT i=0; i<_len; i++)
	{
		TCHAR _tempCh = _totupper(str[i]);//str[i];
		if ( setIgnoreCh.HasKey(_tempCh) )
		{
			memcpy((LPVOID)(&(str[i])), (LPVOID)(&(str[i+1])), sizeof(TCHAR)*(_len-i));
			i--;
			_len--;
		}
	}
}

void CavaWordFilter::SetCensorWord(TCHAR *str)
{
	INT _len = _tcslen(str);
	CavaFilterNode* _now = &root;

	for(INT i=0; i<_len; i++)
	{
		TCHAR _tempCh = _totupper(str[i]);//str[i];
		CavaFilterNode *pNext = _now->mapNext.Find(_tempCh);
		if (pNext)
			_now = pNext;
		else
		{
			CavaFilterNode _tempNode(_tempCh);
			_now = &(_now->mapNext.Set(_tempCh, _tempNode));
		}
	}
	_now->end = TRUE;
}

BOOL CavaWordFilter::IsIncludeCurse(TCHAR *str)
{
	INT _len = _tcslen(str);
	CavaFilterNode* _now = NULL;

	for(INT i=0; i<_len; i++)
	{
		_now = &root;
		for(INT j=i; j<_len; j++)
		{
			TCHAR _tempCh = _totupper(str[j]);//str[j];

			// 무시할 문자인지 검사..
			if ( setIgnoreCh.HasKey(_tempCh) )
			{
				continue;
			}

			CavaFilterNode *pNext = _now->mapNext.Find(_tempCh);
			if (pNext)
			{
				_now = pNext;
				if (_now->end)
					return TRUE;
			}
			else
				break;
		}
	}
	return FALSE;
}

BOOL CavaWordFilter::ReplaceCensorWord(TCHAR *str)
{
	INT _len = _tcslen(str);
	CavaFilterNode* _now = NULL;

	INT _start = -1;
	INT _end = -1;

	for(INT i=0; i<_len; i++)
	{
		_start = -1;
		_end = -1;

		// 무시할 문자인지 검사..
		TCHAR _tempCh = _totupper(str[i]);//str[j];
		if ( setIgnoreCh.HasKey(_tempCh) )
		{
			continue;
		}

		_now = &root;
		for(INT j=i; j<_len; j++)
		{
			TCHAR _tempCh = _totupper(str[j]);//str[j];
			
			// 무시할 문자인지 검사..
			if ( setIgnoreCh.HasKey(_tempCh) )
			{
				continue;
			}

			CavaFilterNode *pNext = _now->mapNext.Find(_tempCh);
			if (pNext)
			{
				if(_start == -1)
					_start = i;

				_now = pNext;
				if(_now->end)
				{
					_end = j;
					break;
				}
			}
			else
				break;
		}

		if(_start != -1 && _end != -1)
		{
			_tcsnset(&(str[_start]),TEXT('*'),_end-_start+1);
		}
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TickerMsg

CavaTickerMsg::CavaTickerMsg() : bInit(FALSE)
{
	if ( !Init() )
	{
		//__asm { int 3 }
	}
}

CavaTickerMsg::~CavaTickerMsg()
{
}

UBOOL CavaTickerMsg::Init()
{
	const BOOL bEncrypted = TRUE;

	bInit = FALSE;

	FString	_Path;
	GConfig->GetString(CFG_SECTION, CFG_TICKERMSG, _Path, GNetIni);
	if (_Path == TEXT(""))
	{
		_Path = TEXT("ticker.txt");
	}

	TArray<FString> Lines;

	if ( LoadStringsFromFile(Lines, (TCHAR*)*_Path, bEncrypted) == 0 )
	{
		_LOG(TEXT("Error! Failed to load ticker message list; file = %s"), *_Path);
		return FALSE;
	}

	for (INT i = 0; i < Lines.Num(); ++i)
		AddPlain((TCHAR*)*Lines(i));

	_LOG(TEXT("%d ticker messages loaded from %s"), Num(), *_Path);

	bInit = TRUE;
	return TRUE;
}

void CavaTickerMsg::Dump()
{
#if !FINAL_RELEASE
	for (TStringList::TIterator It(MsgList.GetHead()); It; ++It)
	{
		_DUMP(**It);
	}
#endif
}

void CavaTickerMsg::AddPlain(TCHAR *Msg)
{
	FString Str = Msg;
	MsgList.AddTail(Str);

	if (MsgList.Num() > 10)
		MsgList.RemoveNode(MsgList.GetHead());
}

void CavaTickerMsg::Add(TCHAR *Msg, const INT MsgType)
{
	FString TypedMsg = FString::Printf(TEXT("[%d]%s"), MsgType, Msg);
	AddPlain((TCHAR*)*TypedMsg);
}

void CavaTickerMsg::Clear()
{
	MsgList.Clear();
}

//UBOOL CavaNetStateController::TickerSave()
//{
//	if (!TickerMsgList.bDirty)
//		return TRUE;
//
//	UINT buflen = 512;
//	BYTE *buf = new BYTE[buflen];
//	appMemzero(buf, buflen);
//	UINT bufpos = 4;
//
//	ANSICHAR _line[1024];
//	for (FTickerMsgList::TStringList::TIterator It(TickerMsgList.MsgList.GetHead()); It; ++It)
//	{
//		INT ret = WideCharToMultiByte(CP_ACP, 0, **It, -1, _line, 1024, NULL, NULL);
//		_line[ret++] = '\r';
//		_line[ret++] = '\n';
//		_line[ret] = 0;
//
//		while (bufpos + ret > buflen)
//		{
//			UINT newbuflen = buflen * 2;
//			BYTE *newbuf = new BYTE[newbuflen];
//			appMemzero(newbuf, newbuflen);
//			appMemcpy(newbuf, buf, buflen);
//			delete[] buf;
//			buf = newbuf;
//			buflen = newbuflen;
//		}
//
//		bufpos += ret;
//	}
//
//	*((UINT*)buf) = bufpos;
//
//	CavaAES _aes;
//	BYTE _key[17];
//
//	appMemzero(_key, 17);
//	_aes.BuildKey(_key, 16, Def::VERSION_ENCRYPT_KEY);
//
//	if ( !_aes.Init((const LPBYTE)_key, 16) )
//	{
//		return FALSE;
//	}
//
//	UINT cipherlen;
//	_aes.Encrypt(buf + 4, bufpos - 4, buf + 4, &cipherlen);
//	if (cipherlen <= 0)
//	{
//		return FALSE;
//	}
//
//	UINT writefilelen = cipherlen + 4;
//
//	FILE *_fp = _tfopen(_TICKER_FILENAME, TEXT("w"));
//	if (!_fp)
//		return FALSE;
//
//	UINT wlen = fwrite(buf, sizeof(BYTE), writefilelen, _fp);
//	fclose(_fp);
//
//	return TRUE;
//}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RoomName

CavaRoomName::CavaRoomName() : bInit(FALSE)
{
	if ( !Init() )
	{
	}
}

CavaRoomName::~CavaRoomName()
{
}

UBOOL CavaRoomName::Init()
{
	const BOOL bEncrypted = TRUE;

	bInit = FALSE;

	FString	_Path;
	GConfig->GetString(CFG_SECTION, CFG_ROOMNAME, _Path, GNetIni);
	if (_Path == TEXT(""))
	{
		_Path = TEXT("room.txt");
	}

	TArray<FString> Lines;

	if ( LoadStringsFromFile(Lines, (TCHAR*)*_Path, bEncrypted) == 0 )
	{
		_LOG(TEXT("Error! Failed to load room name list; file = %s"), *_Path);
		return FALSE;
	}

	for (INT i = 0; i < Lines.Num(); ++i)
		RoomNameList.Push(*Lines(i));

	_LOG(TEXT("%d room names loaded from %s"), RoomNameList.Num(), *_Path);

	bInit = TRUE;
	return TRUE;
}

FString CavaRoomName::GetRandomRoomName()
{
	if (RoomNameList.Num() > 0)
	{
		return RoomNameList(appRand() % RoomNameList.Num());
	}

	return TEXT("");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AwardDesc

CavaAwardDesc::CavaAwardDesc()
{
	if ( !Init() )
	{
	}
}

CavaAwardDesc::~CavaAwardDesc()
{
}

UBOOL CavaAwardDesc::Init()
{
	const BOOL bEncrypted =	TRUE;

	bInit = FALSE;

	FString	AwardDescName;
	if ( !GConfig->GetString(CFG_SECTION, CFG_AWARDDESC, AwardDescName, GNetIni) || AwardDescName == TEXT("") )
		AwardDescName = TEXT("AwardDesc.ini");

	CIni _ini(*AwardDescName);

	UBOOL Res = (bEncrypted ? _ini.OpenConfigAES() : _ini.OpenConfig());
	if (Res)
		Res = _ini.SelectFirstSection();

	if(!Res)
	{
		_LOG(TEXT("Error! Failed to load award list; file = %s"), *AwardDescName);
		return FALSE;
	}

	FAwardInfo *AwardInfo;
	do
	{
		AwardInfo = new(AwardList) FAwardInfo();
		check(AwardInfo);

		BYTE id = (TID_AWARD)_ini.GetInteger(TEXT("AwardID"), ID_INVALID_AWARD);
		if (id == ID_INVALID_AWARD)
		{
			_LOG(TEXT("Error! Failed to read AwardID from section [%s]"), _ini.GetCurrentSectionName());
			return FALSE;
		}
		if ( GetAwardInfo(id) != NULL )
		{
			_LOG(TEXT("Error! AwardInfo already exists; section [%s], id = %d"), _ini.GetCurrentSectionName(), id);
			return FALSE;
		}

		AwardInfo->id = id;

		AwardInfo->Grade = (BYTE)_ini.GetInteger(TEXT("AwardGrade"), UCHAR_MAX);
		if (AwardInfo->Grade == UCHAR_MAX)
		{
			_LOG(TEXT("Error! Failed to read AwardGrade from section [%s], id = %d"), _ini.GetCurrentSectionName(), AwardInfo->id);
			return FALSE;
		}

		AwardInfo->Type = (BYTE)_ini.GetInteger(TEXT("AwardType"), UCHAR_MAX);
		if (AwardInfo->Type == UCHAR_MAX)
		{
			_LOG(TEXT("Error! Failed to read AwardType from section [%s], id = %d"), _ini.GetCurrentSectionName(), AwardInfo->id);
			return FALSE;
		}

		AwardInfo->AwardName = _ini.GetString(TEXT("AwardName"), TEXT(""));
		if (AwardInfo->AwardName == TEXT(""))
		{
			_LOG(TEXT("Error! Failed to read AwardName from section [%s], id = %d"), _ini.GetCurrentSectionName(), AwardInfo->id);
			return FALSE;
		}

		AwardInfo->Desc = _ini.GetString(TEXT("AwardDesc"), TEXT("No description"));
		AwardInfo->RewardDesc = _ini.GetString(TEXT("AwardRewardDesc"), TEXT("No description"));

		FString _t = _ini.GetString(TEXT("CheckProperty"), TEXT(""));
		if (_t != TEXT(""))
		{
			AwardInfo->CheckProperty = GetCheckProperty(*_t);
			AwardInfo->CheckValue = _ini.GetInteger(TEXT("CheckValue"), -1);
		}
		else
		{
			AwardInfo->CheckProperty = _Check_None;
			AwardInfo->CheckValue = -1;
		}
	}
	while(_ini.SelectNextSection());

	_LOG(TEXT("%d award info loaded from %s"), AwardList.Num(), *AwardDescName);

	bInit = TRUE;
	return TRUE;
}

FAwardInfo* CavaAwardDesc::GetAwardInfo(BYTE id)
{
	for (INT i = 0; i < AwardList.Num(); ++i)
	{
		if (AwardList(i).id == id)
		{
			return &AwardList(i);
		}
	}

	return NULL;
}

#define _IF_CHECKPROPERTY(_str, _name) if ( _tcsicmp(_str, TEXT(#_name)) == 0 )  { return _Check_##_name; }
#define _ELSEIF_CHECKPROPERTY(_str, _name) else if ( _tcsicmp(_str, TEXT(#_name)) == 0 )  { return _Check_##_name; }

INT CavaAwardDesc::GetCheckProperty(const TCHAR *prop)
{
	_IF_CHECKPROPERTY(prop, Level)
	_ELSEIF_CHECKPROPERTY(prop, XP)
	_ELSEIF_CHECKPROPERTY(prop, GameWin)
	_ELSEIF_CHECKPROPERTY(prop, GameDefeat)
	_ELSEIF_CHECKPROPERTY(prop, RoundWin)
	_ELSEIF_CHECKPROPERTY(prop, RoundDefeat)
	_ELSEIF_CHECKPROPERTY(prop, DisconnectCount)
	_ELSEIF_CHECKPROPERTY(prop, DeathCount)
	_ELSEIF_CHECKPROPERTY(prop, StraightWinCount)
	_ELSEIF_CHECKPROPERTY(prop, TeamKillCount)
	_ELSEIF_CHECKPROPERTY(prop, PlayTime)

	_ELSEIF_CHECKPROPERTY(prop, Score_Attacker)
	_ELSEIF_CHECKPROPERTY(prop, Score_Defender)
	_ELSEIF_CHECKPROPERTY(prop, Score_Leader)
	_ELSEIF_CHECKPROPERTY(prop, Score_Tactic)

	_ELSEIF_CHECKPROPERTY(prop, P_PlayRound)
	_ELSEIF_CHECKPROPERTY(prop, P_HeadshotCount)
	_ELSEIF_CHECKPROPERTY(prop, P_HeadshotKillCount)
	_ELSEIF_CHECKPROPERTY(prop, P_PlayTime)
	_ELSEIF_CHECKPROPERTY(prop, P_SprintTime)
	_ELSEIF_CHECKPROPERTY(prop, P_TakenDamage)
	_ELSEIF_CHECKPROPERTY(prop, P_KillCount)
	_ELSEIF_CHECKPROPERTY(prop, P_WeaponKillCount_Pistol)
	_ELSEIF_CHECKPROPERTY(prop, P_WeaponKillCount_Knife)
	_ELSEIF_CHECKPROPERTY(prop, P_WeaponKillCount_Grenade)
	_ELSEIF_CHECKPROPERTY(prop, P_WeaponKillCount_Primary)
	_ELSEIF_CHECKPROPERTY(prop, P_WeaponDamage_Pistol)
	_ELSEIF_CHECKPROPERTY(prop, P_WeaponDamage_Knife)
	_ELSEIF_CHECKPROPERTY(prop, P_WeaponDamage_Grenade)
	_ELSEIF_CHECKPROPERTY(prop, P_WeaponDamage_Primary)

	_ELSEIF_CHECKPROPERTY(prop, R_PlayRound)
	_ELSEIF_CHECKPROPERTY(prop, R_HeadshotCount)
	_ELSEIF_CHECKPROPERTY(prop, R_HeadshotKillCount)
	_ELSEIF_CHECKPROPERTY(prop, R_PlayTime)
	_ELSEIF_CHECKPROPERTY(prop, R_SprintTime)
	_ELSEIF_CHECKPROPERTY(prop, R_TakenDamage)
	_ELSEIF_CHECKPROPERTY(prop, R_KillCount)
	_ELSEIF_CHECKPROPERTY(prop, R_WeaponKillCount_Pistol)
	_ELSEIF_CHECKPROPERTY(prop, R_WeaponKillCount_Knife)
	_ELSEIF_CHECKPROPERTY(prop, R_WeaponKillCount_Grenade)
	_ELSEIF_CHECKPROPERTY(prop, R_WeaponKillCount_Primary)
	_ELSEIF_CHECKPROPERTY(prop, R_WeaponDamage_Pistol)
	_ELSEIF_CHECKPROPERTY(prop, R_WeaponDamage_Knife)
	_ELSEIF_CHECKPROPERTY(prop, R_WeaponDamage_Grenade)
	_ELSEIF_CHECKPROPERTY(prop, R_WeaponDamage_Primary)

	_ELSEIF_CHECKPROPERTY(prop, S_PlayRound)
	_ELSEIF_CHECKPROPERTY(prop, S_HeadshotCount)
	_ELSEIF_CHECKPROPERTY(prop, S_HeadshotKillCount)
	_ELSEIF_CHECKPROPERTY(prop, S_PlayTime)
	_ELSEIF_CHECKPROPERTY(prop, S_SprintTime)
	_ELSEIF_CHECKPROPERTY(prop, S_TakenDamage)
	_ELSEIF_CHECKPROPERTY(prop, S_KillCount)
	_ELSEIF_CHECKPROPERTY(prop, S_WeaponKillCount_Pistol)
	_ELSEIF_CHECKPROPERTY(prop, S_WeaponKillCount_Knife)
	_ELSEIF_CHECKPROPERTY(prop, S_WeaponKillCount_Grenade)
	_ELSEIF_CHECKPROPERTY(prop, S_WeaponKillCount_Primary)
	_ELSEIF_CHECKPROPERTY(prop, S_WeaponDamage_Pistol)
	_ELSEIF_CHECKPROPERTY(prop, S_WeaponDamage_Knife)
	_ELSEIF_CHECKPROPERTY(prop, S_WeaponDamage_Grenade)
	_ELSEIF_CHECKPROPERTY(prop, S_WeaponDamage_Primary)

	_ELSEIF_CHECKPROPERTY(prop, P_SkillMaster)
	_ELSEIF_CHECKPROPERTY(prop, R_SkillMaster)
	_ELSEIF_CHECKPROPERTY(prop, S_SkillMaster)

	return _Check_None;
}

#undef _IF_CHECKPROPERTY
#undef _ELSEIF_CHECKPROPERTY


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SkillDesc

CavaSkillDesc::CavaSkillDesc()
{
	if ( !Init() )
	{
	}
}

CavaSkillDesc::~CavaSkillDesc()
{
}

UBOOL CavaSkillDesc::Init()
{
	const BOOL bEncrypted =	TRUE;

	bInit = FALSE;

	FString	SkillDescName;
	if ( !GConfig->GetString(CFG_SECTION, CFG_SKILLDESC, SkillDescName, GNetIni) || SkillDescName == TEXT("") )
		SkillDescName = TEXT("SkillDesc.ini");

	CIni _ini(*SkillDescName);

	UBOOL Res = (bEncrypted ? _ini.OpenConfigAES() : _ini.OpenConfig());
	if (Res)
		Res = _ini.SelectFirstSection();

	if(!Res)
	{
		_LOG(TEXT("Error! Failed to load skill list; file = %s"), *SkillDescName);
		return FALSE;
	}

	FSkillInfo *SkillInfo;
	FString ClassCode;
	INT ClassID = _CLASS_NONE;
	do
	{
		ClassCode = _ini.GetString(TEXT("SkillClass"), TEXT(""));
		ClassID = (ClassCode == TEXT("P") ? _CLASS_POINTMAN : ClassCode == TEXT("R") ? _CLASS_RIFLEMAN : ClassCode == TEXT("S") ? _CLASS_SNIPER : _CLASS_NONE);
		if (ClassID == _CLASS_NONE)
		{
			_LOG(TEXT("Error! Failed to read SkillClass from section [%s]"), _ini.GetCurrentSectionName());
			return FALSE;
		}

		SkillInfo = new(SkillList[ClassID]) FSkillInfo();
		check(SkillInfo);

		BYTE id = (BYTE)_ini.GetInteger(TEXT("SkillID"), ID_INVALID_SKILL);
		if (id == ID_INVALID_SKILL)
		{
			_LOG(TEXT("Error! Failed to read SkillID from section [%s]"), _ini.GetCurrentSectionName());
			return FALSE;
		}
		if ( GetSkillInfo(ClassID, id) != NULL )
		{
			_LOG(TEXT("Error! SkillInfo already exists; section [%s], id = %d"), _ini.GetCurrentSectionName(), id);
			return FALSE;
		}

		SkillInfo->id = id;

		SkillInfo->TypeID = (BYTE)_ini.GetInteger(TEXT("SkillTypeID"), UCHAR_MAX);
		if (SkillInfo->TypeID == UCHAR_MAX)
		{
			_LOG(TEXT("Error! Failed to read SkillTypeID from section [%s], id = %d"), _ini.GetCurrentSectionName(), SkillInfo->id);
			return FALSE;
		}

		SkillInfo->SkillName = _ini.GetString(TEXT("SkillName"), TEXT(""));
		if (SkillInfo->SkillName == TEXT(""))
		{
			_LOG(TEXT("Error! Failed to read SkillName from section [%s], id = %d"), _ini.GetCurrentSectionName(), SkillInfo->id);
			return FALSE;
		}

		SkillInfo->TypeName = _ini.GetString(TEXT("SkillTypeName"), TEXT(""));
		if (SkillInfo->TypeName == TEXT(""))
		{
			_LOG(TEXT("Error! Failed to read TypeName from section [%s], id = %d"), _ini.GetCurrentSectionName(), SkillInfo->id);
			return FALSE;
		}

		SkillInfo->CondDesc = _ini.GetString(TEXT("SkillCondDesc"), TEXT("No description"));
		SkillInfo->EffectDesc = _ini.GetString(TEXT("SkillEffectDesc"), TEXT("No description"));
	}
	while(_ini.SelectNextSection());

	_LOG(TEXT("[%d/%d/%d] skill info loaded from %s"), SkillList[0].Num(), SkillList[1].Num(), SkillList[2].Num(), *SkillDescName);

	bInit = TRUE;
	return TRUE;
}

FSkillInfo* CavaSkillDesc::GetSkillInfo(BYTE idClass, BYTE id)
{
	if (idClass >= 0 && idClass < _CLASS_MAX)
	{
		for (INT i = 0; i < SkillList[idClass].Num(); ++i)
		{
			if (SkillList[idClass](i).id == id)
			{
				return &SkillList[idClass](i);
			}
		}
	}

	return NULL;
}
