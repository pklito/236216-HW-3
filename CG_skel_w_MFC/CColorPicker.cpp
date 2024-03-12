// CColorPicker.cpp : implementation file
//
#include "stdafx.h"
#include "CColorPicker.h"
#include "CG_skel_w_MFC.h"
#include "afxdialogex.h"


// CColorPicker dialog

IMPLEMENT_DYNAMIC(CColorPicker, CDialogEx)

CColorPicker::CColorPicker(CWnd* pParent /*=nullptr*/, Light* light)
	: CDialogEx(IDD_DIALOG3, pParent), light(light)
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
	DDX_Control(pDX, IDC_EDIT1, m_subtitle);
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
	
	if(light){
		m_subtitle.SetWindowText(_T("AMONGUS"));
		vec3 col = light->getColor();
		m_color.SetColor(RGB(col.x*255, col.y*255,col.z*255));
		m_slider.SetPos(int(light->getIntensity()*100));
	}
	
	return TRUE;
}