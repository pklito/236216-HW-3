// CColorPicker.cpp : implementation file
//
#include "stdafx.h"
#include "CColorPicker.h"
#include "CG_skel_w_MFC.h"
#include "afxdialogex.h"


// CColorPicker dialog

IMPLEMENT_DYNAMIC(CColorPicker, CDialogEx)

CColorPicker::CColorPicker(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG3, pParent)
	, m_sliderval(0)
{

}

CColorPicker::~CColorPicker()
{
}

void CColorPicker::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, m_slider);
	DDX_Control(pDX, IDC_MFCCOLORBUTTON1, m_color);
	DDX_Slider(pDX, IDC_SLIDER1, m_sliderval);
}


BEGIN_MESSAGE_MAP(CColorPicker, CDialogEx)
END_MESSAGE_MAP()


// CColorPicker message handlers


void CColorPicker::OnNMCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = m_slider.GetPos();
}


BOOL CColorPicker::OnInitDialog(){
	CDialogEx::OnInitDialog();
	m_slider.SetRange(0,300);
	 m_slider.SetPos(100);
	 m_slider.SetTic(100);
	return TRUE;
}