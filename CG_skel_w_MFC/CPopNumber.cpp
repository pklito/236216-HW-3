// CPopNumber.cpp : implementation file
//
#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "afxdialogex.h"
#include "CPopNumber.h"


// CPopNumber dialog

IMPLEMENT_DYNAMIC(CPopNumber, CDialogEx)

CPopNumber::CPopNumber(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG4, pParent)
	, m_inputval(_T(""))
{

}

CPopNumber::CPopNumber(CString header, CString default_value): CDialogEx(IDD_DIALOG4, nullptr), header(header), val(default_value){
	
}

CPopNumber::~CPopNumber()
{
}

void CPopNumber::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_header1);
	DDX_Control(pDX, IDC_EDIT2, m_input);
	DDX_Text(pDX, IDC_EDIT2, m_inputval);
}


BOOL CPopNumber::OnInitDialog(){
	CDialogEx::OnInitDialog();
	m_header1.SetWindowText(header);
	m_input.SetWindowText(val);
	return TRUE;
}


BEGIN_MESSAGE_MAP(CPopNumber, CDialogEx)
END_MESSAGE_MAP()


// CPopNumber message handlers
