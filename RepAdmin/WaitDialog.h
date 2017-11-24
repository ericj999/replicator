#pragma once


// WaitDialog dialog

class WaitDialog : public CDialogEx
{
	DECLARE_DYNAMIC(WaitDialog)

public:
	WaitDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~WaitDialog();

// Dialog Data
	enum { IDD =  IDD_WAITING};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
};
