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
	, m_sliderval(0)
{

}

CPopup::~CPopup()
{
}

void CPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_msg1); // Map the member variable to the edit control
	DDX_Text(pDX, IDC_EDIT2, m_msg2);
	DDX_Text(pDX, IDC_EDIT3, m_msg3);

	DDX_Control(pDX, IDC_SLIDER1, m_slider);

	DDX_Slider(pDX, IDC_SLIDER1, m_sliderval);
}

BOOL CPopup::OnInitDialog(){
	CDialogEx::OnInitDialog();
	GetDlgItem(IDC_EDIT1)->SetWindowText(_T("1")); // Replace "Default Text" with your desired default text
	GetDlgItem(IDC_EDIT2)->SetWindowText(_T("0.5")); // Replace "Default Text" with your desired default text
	GetDlgItem(IDC_EDIT3)->SetWindowText(_T("5")); // Replace "Default Text" with your desired default text

	 m_slider.SetRange(30,160);
	 m_slider.SetPos(70);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CPopup, CDialogEx)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CPopup::OnNMCustomdrawSlider1)
END_MESSAGE_MAP()


// CPopup message handlers


void CPopup::OnNMCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	//m_sliderval = m_slider.GetPos();
	//*pResult = m_slider.GetPos();
}


/////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

// CPopupOrtho dialog

IMPLEMENT_DYNAMIC(CPopupOrtho, CDialogEx)

CPopupOrtho::CPopupOrtho(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{

}

CPopupOrtho::~CPopupOrtho()
{
}

void CPopupOrtho::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_msg1); // Map the member variable to the edit control
	DDX_Text(pDX, IDC_EDIT2, m_msg2);
	DDX_Text(pDX, IDC_EDIT3, m_msg3);
	DDX_Text(pDX, IDC_EDIT4, m_msg4);
}

BOOL CPopupOrtho::OnInitDialog(){
	CDialogEx::OnInitDialog();
	GetDlgItem(IDC_EDIT1)->SetWindowText(_T("2")); // Replace "Default Text" with your desired default text
	GetDlgItem(IDC_EDIT2)->SetWindowText(_T("2")); // Replace "Default Text" with your desired default text
	GetDlgItem(IDC_EDIT3)->SetWindowText(_T("0.5")); // Replace "Default Text" with your desired default text
	GetDlgItem(IDC_EDIT4)->SetWindowText(_T("5")); // Replace "Default Text" with your desired default text


	return TRUE;
}
BEGIN_MESSAGE_MAP(CPopupOrtho, CDialogEx)
END_MESSAGE_MAP()


// CPopupOrtho message handlers