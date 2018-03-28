#if !defined(AFX_BOOKMARKMANAGER_H__F5563586_8E2F_45B1_B6B7_135E48551343__INCLUDED_)
#define AFX_BOOKMARKMANAGER_H__F5563586_8E2F_45B1_B6B7_135E48551343__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BookmarkManager.h : header file
//

#include "ListCtrlEx.h"

/////////////////////////////////////////////////////////////////////////////
// CBookmarkManager dialog
struct TRACE_BOOKMARK;
typedef list<TRACE_BOOKMARK*> CListOfNewBM;

typedef enum enumBmMatch
{
	eDifferent,
	eIdentical,
	eSameEvent
};

class CBookmarkManager : public CDialog
{
// Construction
public:
	CBookmarkManager(double TS=0, CWnd* pParent = NULL);   // standard constructor
    virtual ~CBookmarkManager();
		
// Dialog Data
	//{{AFX_DATA(CBookmarkManager)
	enum { IDD = IDD_MANAGE_BOOKMARKS };
	CListCtrlEx	m_bmList;
	CString	m_comment;
	CString	m_bmName;
	//}}AFX_DATA
       
private:
    CMenu               m_Menu;
    CStringArray        m_ChannelNames;
    
    bool                m_bBMDelta;
    CTraceDataWrapper*  m_pTDIO;
    CTraceDataWrapper*  m_pTDIODV;
    BYTE*               m_pEventForward;
    BYTE*               m_pEventBackward;
	int					m_nNextRowToDisplay;
    
    void                _ApplyChanges();
	void                EnableButtons(bool);
	void                EnableExportMenu();
    void                _EnableControls(bool bEnable, bool bNameChange=false);
    bool                _CreateBookMarkFromTimeStamp(TRACE_BOOKMARK *pCurBM);
	enumBmMatch			AreBookmarksSame(TRACE_BOOKMARK* pBM1, TRACE_BOOKMARK* pBM2);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBookmarkManager)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	//}}AFX_VIRTUAL

    void UpdateBookMarkDetails(int nRow);
    void InitializeLocalLists();

// Implementation
protected:
	double m_curTS;
	// Generated message map functions
	//{{AFX_MSG(CBookmarkManager)
	virtual BOOL OnInitDialog();
	afx_msg void OnBmDelete();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBmDeleteAll();
	afx_msg void OnKeydownBmList(NMHDR* pNMHDR, LRESULT* pResult) ;
	afx_msg void OnChangeBmName();
	afx_msg void OnChangeBmComment();
	afx_msg void OnFileExportbookmarks();
	afx_msg void OnFileImportbookmarks();
	afx_msg void OnApply();
	afx_msg void OnKillfocusBmName();
	afx_msg void OnItemchangedBmList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	
private:
	int GetBookmarkIndex(double TS);
	int GetCurSel();
	void ClearAllBMs();
	void SaveColWidths();

	//Rammohan Kotturu
	//This method clips given time in microseconds to up to three decimal places (nanosecs).
	double ClipToNanoSec(double microSeconds);

	bool IsDuplicateBMName();
	void OnEmptyBMList();
	DECLARE_MESSAGE_MAP()

    vector<TRACE_BOOKMARK*>m_arpBookmarks;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOOKMARKMANAGER_H__F5563586_8E2F_45B1_B6B7_135E48551343__INCLUDED_)
