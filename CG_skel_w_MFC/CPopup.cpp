// CPopup.cpp : implementation file
//

#include "stdafx.h"
#include "CPopup.h"
#include "CG_skel_w_MFC.h"
#include "afxdialogex.h"


// CPopup dialog

IMPLEMENT_DYNAMIC(CPopup, CDialogEx)

CPopup::CPopup(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CPopup::~CPopup()
{
}

void CPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT1, m_msg1); // Map the member variable to the edit control
}


BEGIN_MESSAGE_MAP(CPopup, CDialogEx)
END_MESSAGE_MAP()


// CPopup message handlers
