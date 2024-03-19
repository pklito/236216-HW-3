#pragma once
#include "afxdialogex.h"


// CPopNumber dialog

class CPopNumber : public CDialogEx
{
	DECLARE_DYNAMIC(CPopNumber)

public:
	CPopNumber(CWnd* pParent = nullptr);   // standard constructor
	CPopNumber(CString header, CString default_value=_T(""));
	virtual ~CPopNumber();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG4 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL CPopNumber::OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CString header;
	CString val;
	CStatic m_header1;
	CEdit m_input;
	CString m_inputval;
};
