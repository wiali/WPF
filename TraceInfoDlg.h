#if !defined(AFX_TRACEINFODLG_H__477A865E_A2EE_4E54_B461_B2C85B966D53__INCLUDED_)
#define AFX_TRACEINFODLG_H__477A865E_A2EE_4E54_B461_B2C85B966D53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TraceInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTraceInfoDlg dialog

class CTraceInfoDlg : public CDialog
{
// Construction
public:
	CTraceInfoDlg(CDomainManager *pDomainManager, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTraceInfoDlg)
	enum { IDD = IDD_TRACEINFO_DLG };
	CComboBox	m_portsCombo;
	//}}AFX_DATA

private:
	CDomainManager *m_pDomainManager;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTraceInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTraceInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePortnameCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRACEINFODLG_H__477A865E_A2EE_4E54_B461_B2C85B966D53__INCLUDED_)
