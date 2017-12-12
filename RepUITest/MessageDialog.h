#pragma once
#include "resource.h"

// MessageDialog dialog

class MessageDialog : public CDialogEx
{
	DECLARE_DYNAMIC(MessageDialog)

public:
	MessageDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~MessageDialog();

// Dialog Data
	enum { IDD = IDD_MESSAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_messageText;
};

void ShowMessage(LPCTSTR msg, CWnd* parent);
