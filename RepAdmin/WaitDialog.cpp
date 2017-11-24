// WaitDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Replicator.h"
#include "WaitDialog.h"
#include "afxdialogex.h"


// WaitDialog dialog

IMPLEMENT_DYNAMIC(WaitDialog, CDialogEx)

WaitDialog::WaitDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(WaitDialog::IDD, pParent)
{

}

WaitDialog::~WaitDialog()
{
}

void WaitDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(WaitDialog, CDialogEx)
END_MESSAGE_MAP()


// WaitDialog message handlers


void WaitDialog::OnOK()
{
}


void WaitDialog::OnCancel()
{
}
