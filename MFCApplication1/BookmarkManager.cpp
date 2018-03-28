#include "stdafx.h"
#include "gtxtraceviewer.h"
#include "BookmarkManager.h"
#include "BookmarkCombo.h"
#include "BookMarkImportConfirmDlg.h"
#include "DialogsOptionsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CBookmarkManager::CBookmarkManager(double TS /*=0*/, CWnd* pParent /*=NULL*/)
    : CDialog(CBookmarkManager::IDD, pParent)
{
    m_curTS = TS;
    m_pTDIO = NULL;
    m_pTDIODV = NULL;
    //{{AFX_DATA_INIT(CBookmarkManager)
    m_comment = _T("");
    m_bmName = _T("");
    //}}AFX_DATA_INIT

    m_pEventForward = new BYTE[0x10000];
    m_pEventBackward = new BYTE[0x10000];

    CTraceDataWrapper* pTDIO;

    for (int i(0);i<2;i++) {
        pTDIO = new CTraceDataWrapper(0x1000,
                                        THREAD_PRIORITY_LOWEST,
                                        THREAD_PRIORITY_LOWEST,
                                        MAX_GTX_EVENT_SIZE,
                                        0x1000,
                                        AfxGetMainWnd()->m_hWnd,
                                        true,
                                        i==0?TDIO::MainView:TDIO::DWORDView);           //normal grid view
        pTDIO->DisableTheHistogram(); // This CTraceData will never ask for a histogram

        CGTXTraceViewerApp *pApp = (CGTXTraceViewerApp*)AfxGetApp();
        HRESULT hResult1 = pTDIO->OpenDomain(pApp->m_cmdInfo.GetFullDomainName());

        if (S_OK == hResult1) {
            pTDIO->OptimizeLookaheadBuffer(); // must call this after we know how many channels we have. helps performance of 16 channel seeks.

            bool bIsReady = false;
            do {
                pTDIO->AreAllChannelsEventReady (bIsReady);
                if (!bIsReady)
                    Sleep(10);
                } while (!bIsReady);

            pTDIO->GetActualTDIO()->RegisterNewBookmarkCallbackFunction (CGTXTraceViewerApp::OnBookmarksChanged);
            }
        else {
            delete pTDIO;
            pTDIO = NULL;

            return;
            }

        if (i==0)
            m_pTDIO = pTDIO;
        else
            m_pTDIODV = pTDIO;
        }

    return;
    }

CBookmarkManager::~CBookmarkManager()
{
    delete [] m_pEventForward;
    delete [] m_pEventBackward;

    if (m_pTDIO) {
        m_pTDIO->Close();
        delete m_pTDIO;
        }

    if (m_pTDIODV) {
        m_pTDIODV->Close();
        delete m_pTDIODV;
        }

    //delete all the bookmarks from the array
    ClearAllBMs();
}

void CBookmarkManager::ClearAllBMs()
{
    vector<TRACE_BOOKMARK*>::iterator itr;
    for(itr = m_arpBookmarks.begin(); itr != m_arpBookmarks.end(); itr++)
    {
        if(*itr != NULL)
            delete *itr;
    }

    m_arpBookmarks.clear();
}

void CBookmarkManager::DoDataExchange(CDataExchange* pDX)
{
    TRACE_BOOKMARK *pBM = (TRACE_BOOKMARK *)m_bmList.GetItemData(GetCurSel(), 0);

    bool bLegacyBookmark(false);
    if (pBM)
    {
        bLegacyBookmark = !pBM->bIs80_208Bookmark();
    }


    CDialog::DoDataExchange(pDX);


    char Comment[256]="";GetDlgItemText( IDC_BM_COMMENT, Comment, sizeof(Comment) );
    char Name[256]="";GetDlgItemText( IDC_BM_NAME, Name, sizeof(Name) );

    if (bLegacyBookmark && (strlen(Name)>31)  && strlen(Comment) > MAX_BM_NAME_LEN)
    {
        _ASSERTE(pBM!= NULL);
        if (MessageBox("Ok to truncate bookmark comment?","Bookmark name/comment too long",MB_ICONHAND|MB_YESNO|MB_DEFBUTTON1)==IDYES)
        {
            pBM->Upgrade();
            bLegacyBookmark = false;
            SetDlgItemText( IDC_BM_COMMENT, pBM->Comment().c_str() );
        }
        else
        {
            Name[31]=0;
            SetDlgItemText( IDC_BM_NAME, Name );
        }
    }


    //{{AFX_DATA_MAP(CBookmarkManager)
    DDX_Control(pDX, IDC_BM_LIST, m_bmList);
    DDX_Text(pDX, IDC_BM_COMMENT, m_comment);
    DDV_MaxChars(pDX, m_comment, bLegacyBookmark? MAX_LEGACY_BM_COMMENT_LEN : MAX_BM_COMMENT_LEN);
    DDX_Text(pDX, IDC_BM_NAME, m_bmName);
    DDV_MaxChars(pDX, m_bmName,  MAX_BM_NAME_LEN);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBookmarkManager, CDialog)
    //{{AFX_MSG_MAP(CBookmarkManager)
    ON_BN_CLICKED(IDC_BM_DELETE, OnBmDelete)
    ON_BN_CLICKED(IDC_BM_DELETE_ALL, OnBmDeleteAll)
    ON_NOTIFY(LVN_KEYDOWN, IDC_BM_LIST, OnKeydownBmList)
    ON_EN_CHANGE(IDC_BM_COMMENT, OnChangeBmComment)
    ON_COMMAND(ID_FILE_EXPORTBOOKMARKS, OnFileExportbookmarks)
    ON_COMMAND(ID_FILE_IMPORTBOOKMARKS, OnFileImportbookmarks)
    ON_BN_CLICKED(ID_APPLY, OnApply)
    ON_EN_KILLFOCUS(IDC_BM_NAME, OnKillfocusBmName)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_BM_LIST, OnItemchangedBmList)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBookmarkManager message handlers

void InsertBookMark(CListCtrlEx& m_bmList, int nLocation, TRACE_BOOKMARK* pBM, char* strTS, char* pCName) {

    CCellData Row[3];

    Row[0].m_sText = strTS;
    Row[0].m_uiAttribute = LCEX_RIGHT;
    Row[0].m_vExtraData = (LPVOID)pBM;
    Row[1].m_sText = pBM->Name().c_str();
    Row[2].m_sText = pCName;

    m_bmList.InsertRow(nLocation, 3, Row);

    return;
    }

void InsertBookMark(CListCtrlEx& m_bmList, char* strTS, TRACE_BOOKMARK* pBM, char* pCName)
{
    int nCount(m_bmList.GetItemCount());
    CString String;
    double dBMTS;
    int j(0);
    UINT nCh = -1;
    for (j=0;j<nCount;j++)
    {
        String = m_bmList.GetItemText(j, 0);
        dBMTS = ((TRACE_BOOKMARK*)m_bmList.GetItemData(j, 0))->dTimeStamp;
        nCh = ((TRACE_BOOKMARK*)m_bmList.GetItemData(j, 0))->uiChannelNumber;
        if ((pBM->dTimeStamp < dBMTS) || (pBM->dTimeStamp == dBMTS && pBM->uiChannelNumber < nCh))
        {
            InsertBookMark(m_bmList, j, pBM, strTS, pCName);
            return;
        }
    }

    InsertBookMark(m_bmList, j, pBM, strTS, pCName);
}

BOOL CBookmarkManager::OnInitDialog()
{
    CDialog::OnInitDialog();

    _TCHAR* Header[3];
    Header[0] = _T("Timestamp");
    Header[1] = _T("Name");
    Header[2] = _T("Port");
    m_bmList.SetHeader(Header, 3);

    CRect rect;
    m_bmList.GetWindowRect(&rect);

    m_bmList.SetColWidth(0, AfxGetApp()->GetProfileInt("", ENTRY_BM_MNGR_COLWIDTH_0, int(rect.Width() * 0.2)));
    m_bmList.SetColWidth(1, AfxGetApp()->GetProfileInt("", ENTRY_BM_MNGR_COLWIDTH_1, int(rect.Width() * 0.6)));
    m_bmList.SetColWidth(2, AfxGetApp()->GetProfileInt("", ENTRY_BM_MNGR_COLWIDTH_2, rect.Width() - (m_bmList.GetColumnWidth(0) + m_bmList.GetColumnWidth(1)+5)));

    CGTXTraceViewerApp *pApp = (CGTXTraceViewerApp  *) AfxGetApp();
    unsigned long lPortCount = pApp->m_pDomainManager->GetPortCountWithoutIncompatibleXPTPorts();
    for (unsigned long i=0; i < lPortCount; i++)
        m_ChannelNames.Add(pApp->m_pDomainManager->GetAliasedPortName(i));

    InitializeLocalLists();

    //set up the menu for import/export
    SetMenu(NULL);
    m_Menu.LoadMenu(IDR_BOOKMARK_MANAGER_MENU);
    SetMenu(&m_Menu);

    //check to see if there are any bookmarks, if not then we can't allow exporting
    TRACE_BOOKMARK *pCurBM = pApp->m_pDomainManager->GetFirstUserBookmark(TRUE);
    if (!pCurBM) {
        CMenu* pMenu = m_Menu.GetSubMenu(0);
        pMenu->EnableMenuItem(ID_FILE_EXPORTBOOKMARKS, MF_GRAYED);
        }


    CRect Rect;
    GetMenuItemRect(m_hWnd, m_Menu.m_hMenu, 0, &Rect);

    int nHeight = Rect.Height();
    GetWindowRect(&Rect);
    SetWindowPos(&wndTop, 0, 0, Rect.Width(), Rect.Height()+nHeight, SWP_NOREPOSITION|SWP_NOMOVE|SWP_SHOWWINDOW);
    CenterWindow();

    m_bBMDelta = false;

    bool bRes=m_bmList.GetItemCount()>0?true:false;
    EnableButtons(bRes);

    return FALSE;
    }

void CBookmarkManager::InitializeLocalLists() {

    CGTXTraceViewerApp* pApp = (CGTXTraceViewerApp*)AfxGetApp();

    TRACE_BOOKMARK *pCurBM = pApp->m_pDomainManager->GetFirstUserBookmark (TRUE);
    while (pCurBM) {
        TRACE_BOOKMARK *pBM = new TRACE_BOOKMARK();
        *pBM = *pCurBM;
        m_arpBookmarks.push_back(pBM);
        CString strTimestamp = (BSTR)pApp->m_spWireEventDecoder->DumpTimestamp(pBM->dTimeStamp);
        InsertBookMark(m_bmList, strTimestamp.GetBuffer(0), pBM, m_ChannelNames[pBM->uiChannelNumber].GetBuffer(0));
        pCurBM = pApp->m_pDomainManager->GetNextUserBookmark (TRUE);
        }

    if (m_bmList.GetItemCount()) {
        m_bmList.SetCurrentSelectedRow(0);
        m_bmList.SetCurrentSelectedRow(0);
        m_bmList.SetFocus();
        }

    return;
    }

int CBookmarkManager::GetBookmarkIndex(double TS)
{

    int cnt = m_bmList.GetItemCount();

    for (int i=0; i < cnt; i++)
    {
        TRACE_BOOKMARK *pBM = (TRACE_BOOKMARK *)m_bmList.GetItemData(i, 0);
        if (EQUALTS_1GIG(pBM->dTimeStamp, TS))
        {
            return(i);
        }
    }
    // no match found
    return(0);
}


int CBookmarkManager::GetCurSel ()
{

    return m_bmList.GetCurrentSelectedRow();

}

void CBookmarkManager::UpdateBookMarkDetails(int nRow)
{
    TRACE_BOOKMARK* pBM = (TRACE_BOOKMARK *)m_bmList.GetItemData(nRow, 0);
    SetDlgItemText(IDC_BM_TIMESTAMP, m_bmList.GetCellString(nRow, 0));
    SetDlgItemText(IDC_BM_NAME, m_bmList.GetCellString(nRow, 1));
    SetDlgItemText(IDC_BM_PORT, m_bmList.GetCellString(nRow, 2));
    SetDlgItemText(IDC_BM_COMMENT, pBM->Comment().c_str());

    return;
}

void CBookmarkManager::OnBmDelete() {

    int oldsel = GetCurSel();

    if (oldsel == -1)
        return;

    TRACE_BOOKMARK *pBMToDelete = (TRACE_BOOKMARK *)m_bmList.GetItemData(oldsel, 0);
    vector<TRACE_BOOKMARK*>::iterator itr;
    for(itr = m_arpBookmarks.begin(); itr != m_arpBookmarks.end(); itr++)
    {
        if(*itr == pBMToDelete)
        {
            delete( pBMToDelete );
            m_arpBookmarks.erase(itr);
            break;
        }
    }

    m_bmList.DeleteRow(oldsel);

    if (m_bmList.GetItemCount()) {
        if (oldsel >= m_bmList.GetItemCount())
            oldsel = m_bmList.GetItemCount() - 1;
        m_bmList.SetCurrentSelectedRow(oldsel);
        }
    else {
        EnableButtons(false);
        }

    _EnableControls(true);

    return;
    }

void CBookmarkManager::_EnableControls(bool bEnable, bool bNameChange) {

    if (bNameChange&&bEnable)
        m_bBMDelta = bEnable;
    else
        m_bBMDelta = bEnable;

    GetDlgItem(IDC_BM_LIST)->EnableWindow(bEnable);
    GetDlgItem(IDOK)->EnableWindow(bEnable);
    GetDlgItem(ID_APPLY)->EnableWindow(bEnable);

    return;
    }

void CBookmarkManager::EnableButtons(bool bEnable) {
    GetDlgItem(IDC_BM_DELETE_ALL)->EnableWindow(bEnable);
    GetDlgItem(IDC_BM_DELETE)->EnableWindow(bEnable);
    GetDlgItem(IDC_UPDATE_BTN)->EnableWindow(bEnable);
    GetDlgItem(IDC_BM_NAME)->EnableWindow(bEnable);
    GetDlgItem(IDC_BM_COMMENT)->EnableWindow(bEnable);

    if (!bEnable) {
        SetDlgItemText(IDC_BM_NAME, 0);
        SetDlgItemText(IDC_BM_COMMENT, 0);
        SetDlgItemText(IDC_BM_TIMESTAMP, 0);
        SetDlgItemText(IDC_BM_PORT, 0);
    }
}


void CBookmarkManager::OnEmptyBMList()
{
    EnableButtons(false);
}

void CBookmarkManager::OnOK()
{
    _ApplyChanges();
    SaveColWidths();
    CDialog::OnOK();
}

void CBookmarkManager::_ApplyChanges()
{
    CGTXTraceViewerApp *pApp = (CGTXTraceViewerApp  *) AfxGetApp();
    CDomainManager *pDomainManager = pApp->m_pDomainManager;

    if (pDomainManager == NULL)
        return;

    if(0 == m_arpBookmarks.size())
    {
        //remove all book marks from the domain manager
        pDomainManager->RemoveAllTDIOUserBookmarks();
    }
    else
    {
        //make a copy of the local array
        vector<TRACE_BOOKMARK*>arpBookmarksTemp;
        vector<TRACE_BOOKMARK*>::iterator itrTemp = m_arpBookmarks.begin();
        while(m_arpBookmarks.end() != itrTemp)
        {
            arpBookmarksTemp.push_back(*itrTemp);
            itrTemp++;
        }

        //for each bookmark in the domain
        TRACE_BOOKMARK *pBM = pApp->m_pDomainManager->GetFirstUserBookmark(TRUE);

        while(NULL != pBM)
        {
            //get the corresponding bookmark from temp array
            vector<TRACE_BOOKMARK*>::iterator itr;
            for (itr = arpBookmarksTemp.begin(); itr != arpBookmarksTemp.end(); itr++)
            {
                if(pBM->uiChannelNumber == (*itr)->uiChannelNumber && pBM->dTimeStamp == (*itr)->dTimeStamp)
                    break;      //match found
            }

            TRACE_BOOKMARK *pBMPrev = NULL;
            if(itr != arpBookmarksTemp.end())
            {
                //match found
                if(0 != strcmp(pBM->Name().c_str(), (*itr)->Name().c_str()) || 0 != strcmp(pBM->Comment().c_str(), (*itr)->Comment().c_str()))
                    pDomainManager->ReplaceBookmark(*itr);                      //it is different update it

                arpBookmarksTemp.erase(itr);
            }
            else    //it is not found in local array
                pBMPrev = pBM;

            pBM = pApp->m_pDomainManager->GetNextUserBookmark(TRUE);

            if(NULL != pBMPrev)
                pDomainManager->RemoveBookmark(pBMPrev);    //delete it from domain manager

        }
    }

    pApp->UpdateBookMarks();

    m_bBMDelta = false;
}

void CBookmarkManager::OnCancel()
{
    SaveColWidths();
    CDialog::OnCancel();
}

bool CBookmarkManager::IsDuplicateBMName()
{
    if ((GetCurSel() < 0) || m_bmName.IsEmpty())
        return false;

    TRACE_BOOKMARK* pBM = (TRACE_BOOKMARK*) m_bmList.GetItemData (GetCurSel(), 0);
    if (pBM && pBM->Name().c_str() == m_bmName)
        return false;

    int cnt = m_bmList.GetItemCount();
    bool bRet = false;

    for (int i=0; i < cnt; i++)
    {
        if (GetCurSel() == i)
            continue;

        pBM = (TRACE_BOOKMARK *)m_bmList.GetItemData(i, 0);
        if (m_bmName.CompareNoCase(pBM->Name().c_str()) == 0)
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

void CBookmarkManager::OnBmDeleteAll()
{
    ClearAllBMs();
    m_bmList.DeleteAllRows();
    EnableButtons(false);
    _EnableControls(true);

    return;
}

void CBookmarkManager::OnKeydownBmList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

    if (pLVKeyDow->wVKey == VK_DELETE)
        OnBmDelete();

    *pResult = 0;
}

void CBookmarkManager::OnChangeBmName()
{
    int nCurSel(GetCurSel());

    if (GetCurSel() == -1)
        return;

    UpdateData();
    TRACE_BOOKMARK* pBM = (TRACE_BOOKMARK*) m_bmList.GetItemData(GetCurSel(), 0);
    if (m_bmName == pBM->Name().c_str())
        // nothing has changed
        return;

    pBM->SetName( (LPCTSTR)m_bmName);

    m_bmList.SetCellString(GetCurSel(), 1, m_bmName);

    _EnableControls(!m_bmName.IsEmpty(), true);
}

void CBookmarkManager::OnKillfocusBmName()
{
    int nCurSel(GetCurSel());

    if (GetCurSel() == -1)
        return;

    UpdateData();
    TRACE_BOOKMARK* pBM = (TRACE_BOOKMARK*) m_bmList.GetItemData(GetCurSel(), 0);
    if (m_bmName == pBM->Name().c_str())
    {
        // nothing has changed, don't change the name and update the UI with the row to display
        UpdateBookMarkDetails(m_nNextRowToDisplay);
        return;
    }

    if (m_bmName.IsEmpty())
    {
        // No bookmark name has been entered, don't change the name and update the UI with the row to display
        UpdateBookMarkDetails(m_nNextRowToDisplay);
        return;
    }

    if(0 == m_bmName.CompareNoCase(STR_Trigger))
    {
        CString dispStr;
        dispStr.Format("'%s' is a reserved bookmark. Use some other name", STR_Trigger);
        MessageBox(dispStr, _T("Add/Modify Bookmark"), MB_ICONINFORMATION|MB_OK);
        if (pBM)
            m_bmName = pBM->Name().c_str();
        else
            m_bmName = "";
        UpdateData(FALSE);
        // Update the UI with the row to display
        UpdateBookMarkDetails(m_nNextRowToDisplay);
        return;
    }

    CGTXTraceViewerApp *pApp = (CGTXTraceViewerApp *)AfxGetApp();
    double dTimeStamp=0;
    if (IsDuplicateBMName())
    {
        if (CDialogsOptionsPage::PromptOnDuplicateBM()) {
            CString dispStr;
            dispStr.Format("Bookmark named '%s' already exists.\nDo you want to create a bookmark with same name ?", m_bmName);
            if(MessageBox(dispStr, _T("Add/Modify Bookmark"), MB_ICONINFORMATION|MB_YESNO|MB_DEFBUTTON2)==IDNO)
            {
                m_bmName = pBM->Name().c_str();
                UpdateData(FALSE);
                // Update the UI with the row to display
                UpdateBookMarkDetails(m_nNextRowToDisplay);
                return;
            }
        }
    }

    pBM->SetName( (LPCTSTR)m_bmName);

    m_bmList.SetCellString(GetCurSel(), 1, m_bmName);

    _EnableControls(!m_bmName.IsEmpty(), true);

    // Update the UI with the row to display
    UpdateBookMarkDetails(m_nNextRowToDisplay);
}

void CBookmarkManager::OnChangeBmComment()
{
    if (GetCurSel() == -1)
        return;

    UpdateData();

    TRACE_BOOKMARK *pBM = (TRACE_BOOKMARK*) m_bmList.GetItemData(GetCurSel(), 0);

    if (m_comment == pBM->Comment().c_str())
        // nothing has changed
        return;

    if (!m_comment.IsEmpty())
        pBM->SetComment( (LPCTSTR)m_comment);
    else
        pBM->SetComment("");

    _EnableControls(true);

}

void ConformStringToCSVRules(CString* pString) {

    //these conforming rules are based on
    //http://www.creativyst.com/Doc/Articles/CSV/CSV01.htm

    if (pString->GetLength()==0)
        return;

    pString->TrimLeft();
    pString->TrimRight();

    CString Temp(*pString);

    //first, test for a comma, if one is found make sure this string is flanked by DQs
    if (pString->Find(',')) {
        if ((*pString)[0]!='"') {
            //need to prefix this with a DQ
            Temp.Format("\"%s", *pString);
            }
        if ((*pString)[pString->GetLength()-1]!='"') {
            //need to terminate this with a DQ
            Temp += "\"";
            }
        *pString = Temp;
        }

    if (pString->Find('"')) {
        if ((*pString)[0]!='"') {
            //need to prefix this with a DQ
            Temp.Format("\"%s", *pString);
            }
        if ((*pString)[pString->GetLength()-1]!='"') {
            //need to terminate this with a DQ
            Temp += "\"";
            }
        *pString = Temp;
        }

    int nDQs(0);

    for (int i(0);i<pString->GetLength();i++) {
        if ((*pString)[i]=='"')
            nDQs++;
        }

    if (nDQs%2!=0)
    {
        //at least one of these DQs does not have a partner
        //it is easier to convert all DQ pairs to DQ singles
        //then to convert all DQ singles to DQ doubles
        //finally to remove the first and last DQ
        pString->Replace("\"\"", "\"");
        pString->Replace("\"", "\"\"");

        Temp = pString->GetBuffer(0) + 1;
        *pString = Temp.Left(Temp.GetLength()-1);
        }

    //Rammohan Kotturu
    //Bookmark comment can contain more than one line.
    //check for CR/LF pairs
    //CR/LF should be replaced with LF
    //Otherwise generated csv file cannot be read by Excel correctly.
    pString->Replace("\r\n", "\n");

    return;
    }

void CBookmarkManager::OnFileExportbookmarks(){

    if (m_bBMDelta) {
        int nResult = MessageBox("Changes made to existing bookmarks will be lost and not be exported. Cancel export?", "Export Bookmarks", MB_YESNO|MB_ICONEXCLAMATION);
        if (nResult==IDYES) {
            return;
            }
        }

    CFileDialog Dlg(FALSE, "csv", NULL, OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY, szBookMarkFileFilter, this);

    if (IDOK == Dlg.DoModal()) {
        if ( ((CGTXTraceViewerApp*)AfxGetApp())->m_pDomainManager == NULL)
            return;

        CString FileName = Dlg.GetPathName().GetBuffer(0);

        HANDLE hFile = CreateFile(Dlg.GetPathName(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        //Rammohan Kotturu
        //check for file access failures
        if(INVALID_HANDLE_VALUE == hFile)
        {
            TCHAR szBuf[_MAX_PATH];
            LPVOID lpMsgBuf;
            DWORD dw = GetLastError();

            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL );

            wsprintf(szBuf,
                "Failed to export to file \"%s\".\r\nError: %s",
                Dlg.GetPathName(), lpMsgBuf);
            LocalFree(lpMsgBuf);

            MessageBox(szBuf, "Error", MB_OK);
            return;
        }

        CFile File(hFile);
        CString Buffer;
        CString Name;
        CString Comment;
        CString PortName;

        Buffer.Format("Name, Comment, Port Name, Time Stamp (us), Time Stamp (abs), Type, Record Number, Byte Offset\r\n");
        File.Write(Buffer.GetBuffer(0), Buffer.GetLength());

        TRACE_BOOKMARK *pCurBM = NULL;
        int numBookMarks = m_bmList.GetItemCount();
        for (int l_Cntr = 0;l_Cntr<numBookMarks;l_Cntr++)
        {
            pCurBM = (TRACE_BOOKMARK*)m_bmList.GetItemData(l_Cntr, 0);
            if(!pCurBM){continue;}
            Name = pCurBM->Name().c_str();
            Comment = pCurBM->Comment().c_str();
            PortName = m_ChannelNames[pCurBM->uiChannelNumber].GetBuffer(0);

            ConformStringToCSVRules(&Name);
            ConformStringToCSVRules(&Comment);
            ConformStringToCSVRules(&PortName);

            CGTXTraceViewerApp* pApp = (CGTXTraceViewerApp*)AfxGetApp();
            CString strTimestamp = (BSTR)pApp->m_spWireEventDecoder->DumpTimestamp(pCurBM->dTimeStamp);

            Buffer.Format("%s,%s,%s,%f,%s,%d,%I64u,%u\r\n",
                Name,
                Comment,
                PortName,
                pCurBM->dTimeStamp,
                strTimestamp.GetBuffer(0),
                pCurBM->eTypeFull,
                pCurBM->GetRecordNumber(),
                pCurBM->GetRecordOffset());

            File.Write(Buffer.GetBuffer(0), Buffer.GetLength());
        }
        File.Close();
    }
    return;
}

//Rammohan Kotturu
//bookmark comparison is done at two levels.
//Check whether both bookmarks are on the same event.
//Check whether they are identical.
//Order of fields compared is important.
enumBmMatch CBookmarkManager::AreBookmarksSame(TRACE_BOOKMARK* pBMExisiting, TRACE_BOOKMARK* pBMImported)
{
    if (pBMExisiting->uiChannelNumber!=pBMImported->uiChannelNumber)
        return eDifferent;
    if( 0 != pBMImported->GetRecordNumber())
    {
        //record number exists in csv file
        if(!(pBMExisiting->GetCRecord() == pBMImported->GetCRecord()))
            return eDifferent;
    }
    else if (pBMExisiting->dTimeStamp!=pBMImported->dTimeStamp)
        return eDifferent;

    //check type
    if(pBMExisiting->eTypeFull != pBMImported->eTypeFull)
        return eDifferent;

    //Bookmarks are on the same event.
    //If name and or comment fields are different
    //return eSameEvent;
    if (0 != strcmp(pBMExisiting->Name().c_str(), pBMImported->Name().c_str()))
        return eSameEvent;
    if (0 != strcmp(pBMExisiting->Comment().c_str(), pBMImported->Comment().c_str()))
        return eSameEvent;

    return eIdentical;
}

CString GetNextRecord(char** pPos) {

    char* pStart = *pPos;
    char* pEnd = pStart;

    //Rammohan Kotturu
    //Bookmark comment can contain more than one line.
    //Only LF is used to separate multiple lines in the comment.
    //CR/LF pair indicates the end of the record.
    while (!(*(pEnd - 1) == 0x0D && *pEnd == 0x0A) && *pEnd!=0x00) pEnd++;

    bool bLastRecord(*pEnd==0x00);

    //make CString creation easier
    if (*(pEnd-1)==0x0D)
        *(pEnd-1) = 0;
    else
        *pEnd = 0;

    CString String(pStart);

    //restore original char
    if (!bLastRecord) {
        if (*(pEnd-1)==0x00)
            *(pEnd-1) = 0x0D;
        else
            *pEnd = 0x0A;
        }

    while (*pEnd == 0x0A)
        pEnd++;

    *pPos = pEnd;

    return String;
    }

CString GetItemAdvanceToNextInRecord(CString* pRecord) {

    //first, advance across legal white space
    pRecord->TrimLeft();

    int nCommaLocation(pRecord->Find(','));
    int nDQLocation(pRecord->Find('"'));

    CString Item;

    int nLength(pRecord->GetLength());

    if (nCommaLocation==-1)
        nCommaLocation = nLength+1;

    if (nCommaLocation<nDQLocation||nDQLocation==-1) {
        //no embedded comma
        Item = pRecord->Left(nCommaLocation);
        *pRecord = pRecord->Right(nLength-nCommaLocation-1);
        }
    else {
        //embedded comma or embedded DQ
        //two cases:
        //  "ABC,XYZ",
        //  "ABC""123""XYZ",
        //as well as variations of the above two
        //it is logical then to count the DQ's until a comma is found when the DQ count is even
        int nEnd(nLength);
        int nDQs(0);

        while (nEnd) {
            if ((*pRecord)[nLength-nEnd]=='"')
                nDQs++;

            if ((*pRecord)[nLength-nEnd]==',') {
                if (nDQs%2==0)
                    break;
                }

            nEnd--;
            }

        Item = pRecord->Left(nLength-nEnd);
        *pRecord = pRecord->Right(nLength-(nLength-nEnd)-1);
        }

    //Rammohan Kotturu
    //Bookmark comment can contain more than one line.
    //Only LF was saved in to csv file to make it correctly readable by  Excel
    //LF should be replaced with CR/LF
    Item.Replace("\n", "\r\n");
    return Item;
    }

void Trim(CString* pString) {

    if (pString->GetLength()) {
        pString->TrimLeft();
        pString->TrimRight();

        //need to be selective regarding the DQ's we trim off
        if ((*pString)[0]=='\"')
            *pString = pString->Right(pString->GetLength()-1);

        if ((*pString)[pString->GetLength()-1]=='\"')
            *pString = pString->Left(pString->GetLength()-1);
        }

    return;
    }

int GetPortNumber(CString PortName) {

    CGTXTraceViewerApp *pApp = (CGTXTraceViewerApp  *) AfxGetApp();
    for (long i=0; i < (long)pApp->m_pDomainManager->m_PortCaptureInfos.size(); i++)
        if (!strcmp(PortName, pApp->m_pDomainManager->GetAliasedPortName(i)))
            return i;

    return -1;
    }

bool GetNextBM(TRACE_BOOKMARK* pBM, CString* pRecord) {

    CString Name            = GetItemAdvanceToNextInRecord(pRecord);
    CString Comment         = GetItemAdvanceToNextInRecord(pRecord);
    CString PortName        = GetItemAdvanceToNextInRecord(pRecord);
    CString TimeStampUs     = GetItemAdvanceToNextInRecord(pRecord);
    CString TimeStampAbs    = GetItemAdvanceToNextInRecord(pRecord);
    CString BMType          = GetItemAdvanceToNextInRecord(pRecord);
    CString RecordNum       = GetItemAdvanceToNextInRecord(pRecord);
    CString Offset          = GetItemAdvanceToNextInRecord(pRecord);

    Trim(&Name);
    Trim(&Comment);
    Trim(&TimeStampAbs);
    Trim(&PortName);
    Trim(&TimeStampUs);
    Trim(&BMType);
    Trim(&RecordNum);
    Trim(&Offset);

    //TRACE("%s, %s, %s, %s, %s, %s, %s, %s, %s\n", Name, Comment, PortName, TimeStampAbs, TimeStampUs, PortNumber, BMType, RecordNum, Offset);

    pBM->Reset();
    pBM->eTypeFull = (TRACE_BOOKMARK::ETypeOfBookMark)atoi(BMType);

    pBM->eTypeFull = pBM->eTypeFull==TRACE_BOOKMARK::BOOKMARK_USER?pBM->eTypeFull:pBM->eTypeFull==TRACE_BOOKMARK::BOOKMARK_DWORD_VIEW?pBM->eTypeFull:TRACE_BOOKMARK::BOOKMARK_USER;

    pBM->nVersion = BOOKMARK_VERSION;
    pBM->dTimeStamp = atof(TimeStampUs);
    pBM->uiChannelNumber = GetPortNumber(PortName);
    char *EndPtr = RecordNum.GetBuffer()+RecordNum.GetLength();
    pBM->SetRecordNumberAndOffset(_strtoui64(RecordNum, &EndPtr, 10 ),atoi(Offset));

    if (Comment.GetLength() >= MAX_BM_COMMENT_LEN )
    {
        if (MessageBox(NULL,_T("Bookmark Upgrade required. Is it okay to truncate bookmark comment?"),
            _T("Import Bookmark"),
            MB_ICONHAND|MB_YESNO|MB_DEFBUTTON1)==IDNO)
        {
            return false;
        }
        Comment.Truncate(TRACE_BOOKMARK::BM_CommentLength-1);
    }

    pBM->SetName( Name,true);
    pBM->SetComment( Comment);

    if (TimeStampAbs.GetLength() > 0)  // we have an absolute timestamp based on UTC time available...
    {
        // we will use the absolute timestamp based on UTC time to derive our relative timestamp instead
        // of using the relating timestamp directly.  This way, we can correlate 2 traces that haven't been captured
        // with the same clock, but were started/stopped at the same time.

        // However we can only use this timestamp based on UTC Time if we have the UTC start-of-capture for this trace...
        CGTXTraceViewerApp *pApp = (CGTXTraceViewerApp*)AfxGetApp();
        if (pApp->m_spWireEventDecoder->StartOfCaptureTimeIsInUTC) // we have a start-of-capture time in UTC
        {
            // lets remove the year of the start-of-capture (we assume it is always the same year)!!!
            CString TimestampAbsWithoutYear = TimeStampAbs.Right(TimeStampAbs.GetLength()-TimeStampAbs.Find('/')+2);

            // lets derive a relative timestamp from the timestamp based on UTC...
            double dTimestampDerivedFromStartOfCapture = 0.0;
            FromStrTimestampToDouble(TimestampAbsWithoutYear, (SYSTEMTIME*)pApp->m_spWireEventDecoder->StartOfCaptureTime, dTimestampDerivedFromStartOfCapture);
            if(dTimestampDerivedFromStartOfCapture > 0.0)
            {
                pBM->dTimeStamp = dTimestampDerivedFromStartOfCapture;
                return true;
            }
            else
                return false;
        }
    }

    return true;
    }

void CBookmarkManager::OnFileImportbookmarks() {
    if (m_bBMDelta) {
        int nResult = MessageBox("Changes made to existing bookmarks will be lost during the importation process. Cancel import?", NULL, MB_YESNO|MB_ICONEXCLAMATION);
        if (nResult==IDYES) {
            return;
            }
        }

    CFileDialog Dlg(TRUE, "csv", NULL, OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY, szBookMarkFileFilter, this);

    if (IDOK == Dlg.DoModal()) {
        if ( ((CGTXTraceViewerApp*)AfxGetApp())->m_pDomainManager == NULL)
            return;

        AfxGetApp()->DoWaitCursor(1);

        try {
            multimap<double, TRACE_BOOKMARK> mapImport;
            CFile file;
            CFileException ex;
            if (!file.Open(Dlg.GetPathName(), CFile::modeRead, &ex))
            {
                // complain if an error happened
                // no need to delete the ex object

                TCHAR szError[1024];
                ex.GetErrorMessage(szError, 1024);
                AfxMessageBox(szError);
                AfxGetApp()->DoWaitCursor(-1);
                return;
            }

            int nSize;
            char* pBuffer = new char[nSize = file.GetLength()+10];
            memset(pBuffer, 0, nSize);
            file.Read(pBuffer, nSize-10);
            file.Close();

            char* pPos = pBuffer;

            //read the header...
            CString Record(GetNextRecord(&pPos));

            TRACE_BOOKMARK *pBM = NULL;

            list<TRACE_BOOKMARK>InvalidBookMarkList;

            while (9) {
                Record = GetNextRecord(&pPos);
                if (Record.GetLength())
                {
                    pBM = new TRACE_BOOKMARK();
                    if (GetNextBM(pBM, &Record))
                    {
                        pair<double, TRACE_BOOKMARK>Entry(ClipToNanoSec(pBM->dTimeStamp), *pBM);
                        mapImport.insert(Entry);
                    }
                    else {
                        InvalidBookMarkList.push_back(*pBM);
                    }
                    delete pBM;
                    pBM = NULL;
                }
                else
                    break;
            }

            delete [] pBuffer;

            //now, make comparisons between what we have in our map and what we have for existing book marks
            CGTXTraceViewerApp* pApp = (CGTXTraceViewerApp*)AfxGetApp();
            TRACE_BOOKMARK *pCurBM = pApp->m_pDomainManager->GetFirstUserBookmark(TRUE);
            multimap<double, TRACE_BOOKMARK>::iterator iterLow;
            multimap<double, TRACE_BOOKMARK>::iterator iterHigh;

            if (pCurBM) {
                bool bNoToAll = false;
                while (pCurBM) {
                    //Rammohan Kotturu
                    //There can be multiple bookmarks on different ports with the same timestamp
                    iterLow = mapImport.lower_bound(ClipToNanoSec(pCurBM->dTimeStamp));
                    iterHigh = mapImport.upper_bound(ClipToNanoSec(pCurBM->dTimeStamp));

                    //find out the bookmark corresponding to required channel number
                    for(;iterLow != mapImport.end(); iterLow++)
                    {
                        TRACE_BOOKMARK *pCurBMTemp = &iterLow->second;
                        if(pCurBMTemp->uiChannelNumber == pCurBM->uiChannelNumber)
                            break;
                    }

                    if (iterLow!=mapImport.end())
                    {
                        //this BM already exists
                        //let's see if it is different
                        //if so let the user know
                        enumBmMatch eBmMatch(AreBookmarksSame(pCurBM, &iterLow->second));
                        if (eIdentical == eBmMatch)
                            mapImport.erase(iterLow);
                        else if (eSameEvent == eBmMatch)
                        {
                            if (bNoToAll)
                                mapImport.erase(iterLow);
                            else
                            {
                                //post big dialog
                                CBookMarkImportConfirmDlg Dlg(pCurBM, &iterLow->second);

                                AfxGetApp()->DoWaitCursor(-1);
                                Dlg.DoModal();

                                if (Dlg.m_nSelection==eYES)
                                {
                                    //do nothing, keep it
                                }
                                if (Dlg.m_nSelection==eNO)
                                {
                                    //just remove this one
                                    mapImport.erase(iterLow);
                                }
                                if (Dlg.m_nSelection==eYES_TO_ALL)
                                {
                                    //replace all
                                    AfxGetApp()->DoWaitCursor(1);
                                    break;
                                }
                                if (Dlg.m_nSelection==eNO_TO_ALL)
                                {
                                    //don't keep any duplicates from this point forward
                                    mapImport.erase(iterLow);
                                    bNoToAll = true;
                                }
                                if (Dlg.m_nSelection==eCANCEL)
                                {
                                    //abort the importation
                                    return;
                                }
                                AfxGetApp()->DoWaitCursor(1);
                                }
                            }
                        }

                    pCurBM = pApp->m_pDomainManager->GetNextUserBookmark(TRUE);
                    }
                }

            //clear our local lists
            ClearAllBMs();
            m_bmList.DeleteAllItems();

            //add the bookmarks to be imported
            nSize = mapImport.size();
            multimap<double, TRACE_BOOKMARK>::iterator iter;
            iter = mapImport.begin();

            for (int i=0;i<nSize;i++,iter++) {
                TRACE_BOOKMARK bm = iter->second;

                if (_CreateBookMarkFromTimeStamp(&bm)) {
                    pApp->m_pDomainManager->AddBookmark(&bm);
                }
                else {
                    InvalidBookMarkList.push_back(bm);
                }
            }

            InitializeLocalLists();

            pApp->UpdateBookMarks();

            if (InvalidBookMarkList.size())
            {
                InvalidBookMarkList.sort(TRACE_BOOKMARK::SortByTimestamp);
                CString String("The following bookmark(s) could not be imported:\n\n");

                list<TRACE_BOOKMARK>::const_iterator BMList;
                CString Temp;
                for (BMList=InvalidBookMarkList.begin();BMList!=InvalidBookMarkList.end();++BMList)
                {
                    const TRACE_BOOKMARK* pCurrentBM = &(*BMList);
                    CString strTimestamp = (BSTR)pApp->m_spWireEventDecoder->DumpTimestamp(pCurrentBM->dTimeStamp);
                    Temp.Format("%s, %s, %s, %d, %f\n",
                        pCurrentBM->Name().c_str(),
                        pCurrentBM->Comment().c_str(),
                        strTimestamp.GetBuffer(0),
                        pCurrentBM->uiChannelNumber,
                        pCurrentBM->dTimeStamp);

                    String += Temp;
                }

                MessageBox(String,"Invalid Bookmarks");
            }
        }
        catch (...)
        {
            AfxMessageBox("Only .csv files can be imported.");
            AfxGetApp()->DoWaitCursor(-1);
            return;
        }
    }

    if (m_bmList.GetItemCount() > 0)
        EnableButtons(true);

    EnableExportMenu();


    AfxGetApp()->DoWaitCursor(-1);

    return;
}

void CBookmarkManager::EnableExportMenu() {
    CMenu* pMenu = m_Menu.GetSubMenu(0);
    if (pMenu) {
        TRACE_BOOKMARK *pCurBM = ((CGTXTraceViewerApp*)AfxGetApp())->m_pDomainManager->GetFirstUserBookmark(TRUE);
        pMenu->EnableMenuItem(ID_FILE_EXPORTBOOKMARKS, (NULL != pCurBM) ? MF_ENABLED : MF_GRAYED);
    }
}

void GetSingleEvent(CTraceDataWrapper* m_pIODLL, int& NumEvents, Direction eDirection, EventStruct *ArrayOfEvents) {

    int iArrayCount(0);
    EventStruct **pArray = NULL;
    DWORD CurrentNumEventsWanted = NumEvents;
    HRESULT hr = E_FAIL;
    //int c(0);
    DWORD TimeStart = GetTickCount();

    while (CurrentNumEventsWanted) {
        pArray = NULL;

        iArrayCount = CurrentNumEventsWanted;

        hr = m_pIODLL->_GetEventSeries(iArrayCount, eDirection , (void**)&pArray);

        if (hr & TD_HR_ERROR_FLAG) {
            HRESULT hrtmp(hr);

            if ( (hrtmp != (HRESULT) TD_WAIT_TRY_LATER)) {
                // one of the other unhandled error conditions described in tracedata.h
                //ASSERT(FALSE);
                // cause us to quit getting events
                CurrentNumEventsWanted = iArrayCount;
                }
            else if (!iArrayCount) {
                // No events to get right now - the cache must be empty
                // - so wait a little time for some events to come in
                Sleep(1);
                }
            }

        if (iArrayCount) {
            EventStruct **pArrayOfPointers = pArray;
            int i=0;

            while (i < iArrayCount) {
                EventStruct::DeepCopy(ArrayOfEvents/*[c++]*/, pArrayOfPointers[i++] );
                }

            // now release the Events Back to TraceDataIO.DLL (VERY IMPORTANT!)
            m_pIODLL->_Release(iArrayCount, (void**)pArray);
            // pArray should now be NULL - or at least set it to NULL here
            CurrentNumEventsWanted -= iArrayCount;
            }
        }

    NumEvents = iArrayCount;

    return;
    }

bool CBookmarkManager::_CreateBookMarkFromTimeStamp(TRACE_BOOKMARK *pCurBM) {

    CTraceDataWrapper* pTDIO;

    if (pCurBM->eTypeFull==TRACE_BOOKMARK::BOOKMARK_DWORD_VIEW)
        pTDIO = m_pTDIODV;
    else
        pTDIO = m_pTDIO;

    if (!pTDIO)
        return false;

    int bLookAheadBufLimit = pTDIO->GetLookAheadBufferLimit();

    TDIOChannelList ActivePorts;
    pTDIO->GetAllChannels(ActivePorts);

    if (pCurBM->uiChannelNumber != CHANNEL_DONT_CARE) {
        TDIOChannelList::const_iterator ChanList;

        for (ChanList=ActivePorts.begin();ChanList!=ActivePorts.end();++ChanList) {
            if (*ChanList==pCurBM->uiChannelNumber)
                break;
            }

        if (ChanList==ActivePorts.end())
            return false;

        ActivePorts.clear();
        ActivePorts.push_back(pCurBM->uiChannelNumber);
        }

    pTDIO->SetActiveChannels(ActivePorts);
    pTDIO->RemoveAllComparators();
    pTDIO->SeekToTime(pCurBM->dTimeStamp, FORWARD);
    int nCount(1);

    EventStruct* pEventArrayFwd = (EventStruct*)m_pEventForward;

    GetSingleEvent(pTDIO, nCount, FORWARD, pEventArrayFwd);

    if (nCount==0) {
        //set this timestamp to an arbitrarily large value
        pEventArrayFwd->dStartTimeStamp = DBL_MAX;
        }

    pTDIO->SeekToTime(pCurBM->dTimeStamp, BACKWARD);
    EventStruct* pEventArrayRev = (EventStruct*)m_pEventBackward;

    nCount = 1;

    GetSingleEvent(pTDIO, nCount, BACKWARD, pEventArrayRev);

    if (nCount==0) {
        //set this timestamp to an arbitrarily large value
        pEventArrayRev->dStartTimeStamp = DBL_MAX;
        }

    //now select the event with the closest timestamp
    double dDelta1(fabs(pEventArrayFwd->dStartTimeStamp-pCurBM->dTimeStamp));
    double dDelta2(fabs(pEventArrayRev->dStartTimeStamp-pCurBM->dTimeStamp));

    if (dDelta1<dDelta2) {
        pCurBM->dTimeStamp = pEventArrayFwd->dStartTimeStamp;
        pCurBM->SetRecordNumberAndOffset( pEventArrayFwd->Record.GetRecordNumber(), pEventArrayFwd->Record.GetRecordOffset() );
        pCurBM->uiChannelNumber = pEventArrayFwd->nChanNum;
        }
    else {
        pCurBM->dTimeStamp = pEventArrayRev->dStartTimeStamp;

        pCurBM->SetRecordNumberAndOffset( pEventArrayRev->Record.GetRecordNumber(), pEventArrayRev->Record.GetRecordOffset() );
        pCurBM->uiChannelNumber = pEventArrayRev->nChanNum;
        }

    return true;
    }

void CBookmarkManager::OnApply() {

    _ApplyChanges();

    GetDlgItem(ID_APPLY)->EnableWindow(false);
    EnableExportMenu();

    return;
    }

double CBookmarkManager::ClipToNanoSec(double microSeconds)
{
    double temp = microSeconds * 1000.0;
    double intPart = 0.0;
    modf(temp, &intPart);

    return intPart / 1000.0;
}


void CBookmarkManager::OnItemchangedBmList(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if (pNMListView->uChanged && LVIF_STATE)
    {
        if (pNMListView->uNewState && LVIS_SELECTED)
        {
            // this event gets triggered for every state change, but we only care
            // about which item gets the highlight.
            TRACE_BOOKMARK* pBM = (TRACE_BOOKMARK*)m_bmList.GetItemData(pNMListView->iItem, 0);

            if(NULL != pBM)
            {
                // Keep track of the row that we want to display, we'll need it in OnKillfocusBmName()
                m_nNextRowToDisplay = pNMListView->iItem;
                // Don't update the BM info if the Name edit box has the focus.
                // OnKillfocusBmName() will take care of the update after it processes the name.
                // If we don't do that, the name will be updated with the new one selected before we process it.
                CWnd* pWndFocus = GetFocus();
                if (NULL != pWndFocus)
                {
                    if(pWndFocus->m_hWnd != GetDlgItem(IDC_BM_NAME)->m_hWnd)
                        UpdateBookMarkDetails(m_nNextRowToDisplay);
                }
                else
                    UpdateBookMarkDetails(m_nNextRowToDisplay);
            }
        }
    }

    *pResult = 0;
}

BOOL CBookmarkManager::PreTranslateMessage(MSG *pMsg)
{
    //Rammohan Kotturu
    //if the Enter key is pressed when enditing bookmark name do not close the dialog
    //simply update the name of the bookmark and move the focus to comment field
    //see the bug #6427
    if(WM_KEYDOWN == pMsg->message && ::GetDlgItem(GetSafeHwnd(), IDC_BM_NAME) == pMsg->hwnd && 0x0D == pMsg->wParam)
    {
        GetDlgItem(IDC_BM_COMMENT)->SetFocus();
        return TRUE;
    }
    return CDialog::PreTranslateMessage(pMsg);
}
void CBookmarkManager::SaveColWidths()
{
    WORD colWidth0,colWidth1,colWidth2;
    if(!m_bmList.GetColWidth(0,colWidth0)&&!m_bmList.GetColWidth(1,colWidth1)&&!m_bmList.GetColWidth(2,colWidth2)){
        AfxGetApp()->WriteProfileInt("", ENTRY_BM_MNGR_COLWIDTH_0, colWidth0);
        AfxGetApp()->WriteProfileInt("", ENTRY_BM_MNGR_COLWIDTH_1, colWidth1);
        AfxGetApp()->WriteProfileInt("", ENTRY_BM_MNGR_COLWIDTH_2, colWidth2);
    }
}
