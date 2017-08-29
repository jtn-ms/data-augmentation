// common.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "common.h"

#include <codecvt>
#include <string>

COMMON_API void UnicodeToAnsi(const wchar_t *szUnicode, int len, char *szAnsi)
{
	WideCharToMultiByte(CP_ACP, 0, szUnicode, -1, szAnsi, len, NULL, NULL);
}

COMMON_API void AnsiToUnicode(const char *szAnsi, wchar_t *szUnicode, int len)
{
	MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, szUnicode, len);
}
#if 0
SYSTEMTIME GetFileModifiedTime(const char *szFileName)
{
	WIN32_FILE_ATTRIBUTE_DATA	FileInfo;
	SYSTEMTIME					stUTC, stLocal;

	if (!GetFileAttributesExA(szFileName, GetFileExInfoStandard, (LPVOID)&FileInfo))
	{
		GetSystemTime(&stLocal);
	}
	else
	{
		FileTimeToSystemTime(&FileInfo.ftLastWriteTime, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
	}

	return stLocal;
}

CStringA GetFormattedTimeA(const SYSTEMTIME stTime, BOOL b24Hour)
{
	char	szDate[100], szTime[100];

	GetDateFormatA(LOCALE_USER_DEFAULT, 0, &stTime, NULL, szDate, 100);
	GetTimeFormatA(LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &stTime, b24Hour ? "HH':'mm':'ss" : NULL, szTime, 100);

	return CStringA(szDate) + CStringA(" ") + CStringA(szTime);
}

CStringW GetFormattedTimeW(const SYSTEMTIME stTime, BOOL b24Hour)
{
	WCHAR	szDate[100], szTime[100];

	GetDateFormatW(LOCALE_USER_DEFAULT, 0, &stTime, _T("yyyy'-'MM'-'dd"), szDate, 100);
	GetTimeFormatW(LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &stTime, b24Hour ? _T("HH':'mm':'ss") : NULL, szTime, 100);

	return CStringW(szDate) + CStringW(L" ") + CStringW(szTime);
}

CStringA GetFormattedTimeA(const time_t time, BOOL b24Hour)
{
	SYSTEMTIME	stTime;

	CTime(time).GetAsSystemTime(stTime);

	return GetFormattedTimeA(stTime, b24Hour);
}

CStringW GetFormattedTimeW(const time_t time, BOOL b24Hour)
{
	SYSTEMTIME	stTime;

	CTime(time).GetAsSystemTime(stTime);

	return GetFormattedTimeW(stTime, b24Hour);
}
#endif
COMMON_API std::wstring str2wstr(const char *str)
{
	int strLen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	std::wstring wstr(strLen, 0);
	MultiByteToWideChar(CP_UTF8, 0, str, -1, &wstr[0], strLen);

	return wstr;
}
 
COMMON_API std::string wstr2str(const wchar_t *wstr)
{
	int strLen = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	std::string str(strLen, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], strLen, NULL, NULL);

	return str;
}


