#pragma once


// CGeneralPage dialog

class CGeneralPage : public CDialogEx
{
	DECLARE_DYNAMIC(CGeneralPage)

public:
	CGeneralPage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGeneralPage();

	void Refresh(int newTask, bool force);

// Dialog Data
	enum { IDD = IDD_GENERALPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	int m_CurrentTask;
};
