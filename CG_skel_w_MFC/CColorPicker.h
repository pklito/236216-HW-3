#pragma once
#include "afxdialogex.h"
#include "afxcolorbutton.h"
#include "util.h"
// CColorPicker dialog

class CColorPicker : public CDialogEx
{
	DECLARE_DYNAMIC(CColorPicker)

public:
	CColorPicker(CWnd* pParent = nullptr, Light* light = NULL);   // standard constructor
	virtual ~CColorPicker();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif

protected:
	Light* light;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	BOOL OnInitDialog() override;
	CSliderCtrl m_slider;
	CMFCColorButton m_color;
	int m_sliderval;
	CStatic m_subtitle;
};
