// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__492F7920_7A67_4B91_8317_FC98EFF35370__INCLUDED_)
#define AFX_STDAFX_H__492F7920_7A67_4B91_8317_FC98EFF35370__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>

#include <process.h>
#include <tchar.h>
#include <iostream>
#include "string_t.h"

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

void setShowMsg(bool bShowMsg);
bool getShowMsg();
using namespace std;


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__492F7920_7A67_4B91_8317_FC98EFF35370__INCLUDED_)
