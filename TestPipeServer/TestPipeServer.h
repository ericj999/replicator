
// TestPipeServer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CTestPipeServerApp:
// See TestPipeServer.cpp for the implementation of this class
//

class CTestPipeServerApp : public CWinApp
{
public:
	CTestPipeServerApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CTestPipeServerApp theApp;