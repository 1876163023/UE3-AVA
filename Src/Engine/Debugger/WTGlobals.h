#pragma once
enum {
	CMD_ShowDllForm,
	CMD_EditorCommand,
	CMD_EditorLoadTextBuffer,
	CMD_AddClassToHierarchy,
	CMD_ClearHierarchy,
	CMD_BuildHierarchy,
	CMD_ClearWatch,
	CMD_AddWatch,
	CMD_SetCallback,
	CMD_AddBreakpoint,
	CMD_RemoveBreakpoint,	
	CMD_EditorGotoLine,
	CMD_AddLineToLog,
	CMD_EditorLoadClass,
	CMD_CallStackClear,
	CMD_CallStackAdd,
	CMD_DebugWindowState,
	CMD_ClearAWatch,
	CMD_AddAWatch,
	CMD_LockList,
	CMD_UnlockList,
	CMD_SetCurrentObjectName,
	CMD_GameEnded
};

#define FOPENW _wfopen
#define SPRINTFW wsprintfW
#define FPUTSW fputws
#define FFLUSH fflush
#define FCLOSE fclose
#define FSCANFW fwscanf
#define TOLOWER tolower


#ifndef ASSERT
#define ASSERT(x)
#endif

class WTGlobals
{
	FILE *fpLog;
public:
	WCHAR WT_DLLPATH[1024];
	WCHAR WT_INTERFACEDLL[1024];
	WCHAR WT_WATCHFILE[1024];
	WCHAR WT_GAMEPATH[1024];
	WCHAR WT_GAMESRCPATH[1024];
	WCHAR WT_TARGET[1024];
	WCHAR WT_PORT[1024];
	WTGlobals(LPCWSTR logname = NULL)
	{
		fpLog = NULL;
		ReadINI();
		OpenLog(logname);
	}
	void OpenLog(LPCWSTR logname = NULL)
	{
		if(fpLog)
			FCLOSE(fpLog);
		if(WT_DLLPATH[0] && logname)
		{
			WCHAR logfile[1024];
			SPRINTFW(logfile, L"%s%s.log", WT_DLLPATH, logname);
			fpLog = FOPENW(logfile, L"w");
		}
	}
	void LOG(LPCWSTR txt, LPCWSTR optText = NULL)
	{
		if(fpLog && txt)
		{
			FPUTSW(txt, fpLog);
			if(optText)
			{
				FPUTSW(L", ", fpLog);
				FPUTSW(optText, fpLog);
			}
			FPUTSW(L"\n", fpLog);
			FFLUSH(fpLog);
		}

	}
	void ReadINI()
	{
		LPCWSTR cmdLine = GetCommandLineW();
		WCHAR binPath[512];
		GetFilePath(cmdLine, binPath);
		if(!WStrStrI(cmdLine, L"WTUnrealDebuggerSDK.dll") && !WStrStrI(cmdLine, L"tst1.exe"))
			WStrCat(binPath, L"\\WTDebugger");
		WCHAR iniFile[512];
		WStrCpy(iniFile, binPath);
		WStrCat(iniFile, L"\\WTDebugger.ini");
		FILE *fp = FOPENW(iniFile, L"r");
		if(fp)
		{
			FSCANFW(fp, L"%s\r\n", WT_DLLPATH);
			FSCANFW(fp, L"%s\r\n", WT_INTERFACEDLL);
			FSCANFW(fp, L"%s\r\n",  WT_GAMESRCPATH);
			FSCANFW(fp, L"%s\r\n", WT_WATCHFILE);
			FSCANFW(fp, L"%s\r\n", WT_GAMEPATH);
			FSCANFW(fp, L"%s\r\n", WT_GAMESRCPATH);
			FSCANFW(fp, L"%s\r\n", WT_TARGET);
			FSCANFW(fp, L"%s\r\n", WT_PORT);
			FCLOSE(fp);
		}

	}
	//////////////////////////////////////////////////////////////////////////
	// Standard string commands
	int WStrCmpI(LPCWSTR s1, LPCWSTR s2)
	{
		for(int i = 0; s1[i] && s2[i]; i++)
			if(TOLOWER(s2[i]) != TOLOWER(s1[i]))
				return s2[i] - s1[i];
		return 0;
	}
	LPCWSTR WStrStrI(LPCWSTR str, LPCWSTR substr)
	{
		for(int i = 0; str[i]; i++)
			if(!WStrCmpI(&str[i], substr))
				return &str[i];
		return NULL;
	}
	void WStrCpy(LPWSTR to, LPCWSTR from)
	{
		int i = 0;
		for(; from[i]; i++)
			to[i] = from[i];
		to[i] = '\0';
	}
	void WStrCat(LPWSTR to, LPCWSTR appendStr)
	{
		int len = 0;
		for(; to[len]; len++); // get len
		WStrCpy(&to[len], appendStr);
	}
	void GetFilePath(LPCWSTR file, LPWSTR path)
	{
		LPWSTR lastSlash = path;
		if(file[0] == '"')
			file++;
		for(int i = 0; i < 512 && file[i];i++)
		{
			path[i] = file[i];
			if(file[i] == '\\' || file[i] == '/')
				lastSlash = &path[i];
			if(file[i] == '"')
				break;
			if(file[i] == ' ' && file[i+1] == '-')
				break;
		}
		lastSlash[0] = '\0';
	}
	//////////////////////////////////////////////////////////////////////////
};
#define LOG g_WTGlobals.LOG

extern WTGlobals g_WTGlobals;

