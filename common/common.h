#pragma once
#include "dirent.h"
#include <string>
using namespace std;

// integer types
typedef unsigned char		uint8;
typedef char				int8;
typedef unsigned short		uint16;
typedef short				int16;
typedef unsigned long		uint32;
typedef long				int32;
typedef unsigned long long	uint64;
typedef long long			int64;

#ifndef tstring
#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif
#endif

#define APP_USE_UTF8

#ifdef COMMON_EXPORTS
#define COMMON_API __declspec(dllexport)
#else
#define COMMON_API __declspec(dllimport)
#endif

#ifdef UNICODE
#define str2tstr	str2wstr
#define tstr2str	wstr2str
#endif

#ifdef UNICODE
#define GetFormattedTime	GetFormattedTimeW
#else
#define GetFormattedTime	GetFormattedTimeA
#endif

COMMON_API void UnicodeToAnsi(const wchar_t *szUnicode, int len, char *szAnsi);
COMMON_API void AnsiToUnicode(const char *szAnsi, wchar_t *szUnicode, int len);
COMMON_API std::wstring str2wstr(const char *str);
COMMON_API std::string wstr2str(const wchar_t *wstr);
#if 0
SYSTEMTIME GetFileModifiedTime(const char *szFileName);
CStringA GetFormattedTimeA(const SYSTEMTIME stTime, BOOL b24Hour = FALSE);
CStringW GetFormattedTimeW(const SYSTEMTIME stTime, BOOL b24Hour = FALSE);
CStringA GetFormattedTimeA(const time_t time, BOOL b24Hour = FALSE);
CStringW GetFormattedTimeW(const time_t time, BOOL b24Hour = FALSE);
#endif

