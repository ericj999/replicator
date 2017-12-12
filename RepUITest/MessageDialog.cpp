// MessageDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MessageDialog.h"
#include "afxdialogex.h"


// MessageDialog dialog

IMPLEMENT_DYNAMIC(MessageDialog, CDialogEx)

MessageDialog::MessageDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD, pParent)
	, m_messageText(_T(""))
{

}

MessageDialog::~MessageDialog()
{
}

void MessageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MESSAGE, m_messageText);
}


BEGIN_MESSAGE_MAP(MessageDialog, CDialogEx)
END_MESSAGE_MAP()


// MessageDialog message handlers


void ShowMessage(LPCTSTR msg, CWnd* parent)
{
	MessageDialog dlg(parent);

	dlg.m_messageText = msg;
	dlg.DoModal();
}

