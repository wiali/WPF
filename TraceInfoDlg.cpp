// TraceInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gtxtraceviewer.h"
#include "TraceInfoDlg.h"
#include "..\..\Components\Shared\Xgig_UI_Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTraceInfoDlg dialog


CTraceInfoDlg::CTraceInfoDlg(CDomainManager *pDomainManager, CWnd* pParent /*=NULL*/)
	: CDialog(CTraceInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTraceInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT(pDomainManager != NULL);
	m_pDomainManager = pDomainManager;
}


void CTraceInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTraceInfoDlg)
	DDX_Control(pDX, IDC_PORTNAME_COMBO, m_portsCombo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTraceInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CTraceInfoDlg)
	ON_CBN_SELCHANGE(IDC_PORTNAME_COMBO, OnSelchangePortnameCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTraceInfoDlg message handlers

BOOL CTraceInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (((CGTXTraceViewerApp *)AfxGetApp())->m_cmdInfo.m_traceType == FileTrace)
	{
		CDC *pDC = GetDC();
		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);
		UINT numPixels = 40*tm.tmAveCharWidth;
		char szBuffer[MAX_PATH];
		strncpy(szBuffer, m_pDomainManager->GetTDIOWrapper()->GetCurrentDomainName(), MAX_PATH-1);
		PathCompactPath(pDC->m_hDC, szBuffer, numPixels);
		SetDlgItemText(IDC_TRACE_NAME, szBuffer);
		ReleaseDC(pDC);
	}
	else
		SetDlgItemText(IDC_TRACE_NAME, m_pDomainManager->GetTDIOWrapper()->GetCurrentDomainName());

	SetDlgItemInt(IDC_ACTIVEPORTS, m_pDomainManager->GetTDIOWrapper()->GetNumberOfChannels());

	unsigned long lPortCount = m_pDomainManager->GetPortCountWithoutIncompatibleXPTPorts();
	for (unsigned long i=0; i < lPortCount; i++)
	{
		CString strPortName = m_pDomainManager->GetAliasedPortName (i);
		int comboIndex = m_portsCombo.AddString(strPortName);
		m_portsCombo.SetItemData(comboIndex, i);
	}
	m_portsCombo.SetCurSel(0);
	OnSelchangePortnameCombo();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTraceInfoDlg::OnSelchangePortnameCombo() 
{
	CString sText;
	
	SetDlgItemText(IDC_PROTOCOL, "");
	SetDlgItemText(IDC_SPEED, "");
	SetDlgItemText(IDC_TSV, "");
	SetDlgItemText(IDC_TRACE_SIZE, "");
	SetDlgItemText(IDC_TRACE_VALID, "");
	SetDlgItemText(IDC_TRIGGER_VALID, "");
	SetDlgItemText(IDC_TS_FIRST, "");
	SetDlgItemText(IDC_TS_LAST, "");
	SetDlgItemText(IDC_TS_TRIGGER, "");
	SetDlgItemText(IDC_DIP, "");
	SetDlgItemText(IDC_VERSION3, "");
	
	int i = (int)m_portsCombo.GetItemData(m_portsCombo.GetCurSel());
	CaptureInfo &stCaptureInfo = m_pDomainManager->m_PortCaptureInfos[i];
    
	CGTXTraceViewerApp* pApp = (CGTXTraceViewerApp*)AfxGetApp();
    
    CString DateTime; BOOL bXGIG;
    SYSTEMTIME* pStartTime = (SYSTEMTIME*)pApp->m_spWireEventDecoder->StartOfCaptureTime;
    SYSTEMTIME stopTime = FromTraceDateTimeToSystemTime (stCaptureInfo.sTraceDate, stCaptureInfo.sStopTime, &bXGIG);
    SYSTEMTIME* pStopTime = &stopTime;
    CString TimeReference;
    if (pApp->m_spWireEventDecoder->ConvertUTCStartOfCaptureToLocalTime) 
        TimeReference = "  (LOCAL)";
    else
        TimeReference = "  (UTC)";

    DateTime.Format("%02d#%02d#%d %02d:%02d:%02d %s", 
        pStartTime->wMonth,
        pStartTime->wDay,
        pStartTime->wYear,
        pStartTime->wHour,
        pStartTime->wMinute,
        pStartTime->wSecond,
        TimeReference);

    DateTime.Replace("00#00#0", "??#??#????");
    DateTime.Replace("#", "/");
    
    SetDlgItemText(IDC_STARTTIME, DateTime);
    
    DateTime.Format("%02d#%02d#%d %02d:%02d:%02d %s", 
        pStopTime->wMonth,
        pStopTime->wDay,
        pStopTime->wYear,
        pStopTime->wHour,
        pStopTime->wMinute,
        pStopTime->wSecond,
        TimeReference);

    DateTime.Replace("00#00#0", "??#??#????");
    DateTime.Replace("#", "/");
    
    SetDlgItemText(IDC_STOPTIME, DateTime);

    SetDlgItemText(IDC_PROTOCOL, stCaptureInfo.ProtocolAsString);

	if(NULL != strstr(stCaptureInfo.BitRateAsString, "?.?"))
		SetDlgItemText(IDC_SPEED, "");
	else
		SetDlgItemText(IDC_SPEED, stCaptureInfo.BitRateAsString);

	switch(stCaptureInfo.m_eTrafficSummaryState)
	{
	case eTSSNoTrafficSummary:
		SetDlgItemText(IDC_TSV, "NO");
		break;

	case eTSSHasTrafficSummary:
		SetDlgItemText(IDC_TSV, "YES");
		break;

	case eTSSObsoleteTrafficSummary:
		SetDlgItemText(IDC_TSV, "Invalid");
		break;

	case eTSSBySoftware:
		SetDlgItemText(IDC_TSV, "SW generated");
		break;

	case eTSSDataCorrupted:
		SetDlgItemText(IDC_TSV, "Corrupted");
		break;

	case eTSSMemFull:
		SetDlgItemText(IDC_TSV, "Mem Full");
		break;

	case eTSSUnavailable:
		SetDlgItemText(IDC_TSV, "Not Available");
		break;

#ifdef _DEBUG // As per Rammohan: Only have these detailed descriptions of the fail for our internal debug version of the code.
	case eTSSUnavailableOutOfMemory_ullAvailPhys:

		SetDlgItemText(IDC_TSV, "Out of mem(Phy)");
		break;

	case eTSSUnavailableOutOfMemory_ullAvailVirtual:
		SetDlgItemText(IDC_TSV, "Out of mem(Virtual)");
		break;

	case eTSSUnavailableOutOfMemory_dwMemoryLoad:
		SetDlgItemText(IDC_TSV, "Out of mem(Load)");
		break;

	case eTSSUnavailableOutOfMemory_CMemoryException:
		SetDlgItemText(IDC_TSV, "Out of mem(alloc)");
		break;
#endif   //#ifdef _DEBUG 
	default:
		sText.Format("Out of mem(%d)",stCaptureInfo.m_eTrafficSummaryState);
		SetDlgItemText(IDC_TSV, sText);
		break;
	}

	CString strUnits;
	if(stCaptureInfo.nTraceSize < (1024 * 1024))
	{
		// < 1MB
		//Show capture size in bytes
		sText.Format("%I64d", stCaptureInfo.nTraceSize);
		FormatNumeric (sText);
		strUnits = " Bytes";
	}
	else if(stCaptureInfo.nTraceSize < (1024 * 1024 * 1024))
	{
		//Between 1 MB and 1 GB
		//Show capture size with 1 decimal in MB
		sText.Format("%.1f", ((double)stCaptureInfo.nTraceSize) / (1024 * 1024));
		strUnits = " MB";
	}
	else
	{
		//more than 1 GB
		//Show capture size with 1 decimal in GB
		sText.Format("%.1f", ((double)stCaptureInfo.nTraceSize) / (1024 * 1024 * 1024));
		strUnits = " GB";
	}

	SetDlgItemText(IDC_TRACE_SIZE, sText + strUnits);

	sText.Format("%s", (stCaptureInfo.bTriggerValid) ? "YES":"NO");
	SetDlgItemText(IDC_TRIGGER_VALID, sText);

	CString Version(stCaptureInfo.sTDIOVersion);
    SetDlgItemText(IDC_VERSION3, Version); 
	
	if (!stCaptureInfo.bUTCTime /*local hardware*/ && 
		(_strcmpi(stCaptureInfo.sTDIOVersion, "Unknown") != 0))
   	{
		sText.Format("%d", (stCaptureInfo._cDIPSwitchValue) );
   		SetDlgItemText(IDC_DIP, sText);
		GetDlgItem(IDC_DIP_LABEL)->ShowWindow (SW_SHOW);
		GetDlgItem(IDC_DIP)->ShowWindow (SW_SHOW);
	}
	else // hide the dip switch
	{
		GetDlgItem(IDC_DIP_LABEL)->ShowWindow (SW_HIDE);
		GetDlgItem(IDC_DIP)->ShowWindow (SW_HIDE);
   	}


    CString strFirstTimestamp = (BSTR)pApp->m_spWireEventDecoder->DumpTimestamp(stCaptureInfo.dFirstTimeStamp);
    SetDlgItemText(IDC_TS_FIRST, strFirstTimestamp);

    CString strLastTimestamp = (BSTR)pApp->m_spWireEventDecoder->DumpTimestamp(stCaptureInfo.dLastTimeStamp);
    SetDlgItemText(IDC_TS_LAST, strLastTimestamp);
	
    // Display Capture Wrapped info...
	switch (stCaptureInfo.CaptureWrapStatus)
	{
	case CaptureWrapped:	sText = "YES"; break;
	case CaptureNotWrapped:	sText = "NO"; break;
	default: sText = "N/A"; 
	}
	SetDlgItemText(IDC_WRAPPED, sText);

	if (stCaptureInfo.bTriggerValid)
	{
		CString strTemp;
		CString strTrigTime = (BSTR)pApp->m_spWireEventDecoder->DumpTimestamp(stCaptureInfo.dTriggerTime);
		if(strlen(stCaptureInfo.EnhancedTriggerInfo_TriggerChannelName) > 0)
			strTemp.Format("%s on \"%s\"", strTrigTime, stCaptureInfo.EnhancedTriggerInfo_TriggerChannelName);
		else
			strTemp = strTrigTime;
		SetDlgItemText(IDC_TS_TRIGGER, strTemp);
	}
	else
		SetDlgItemText(IDC_TS_TRIGGER, "Not Set");

	//Rammohan Kotturu
	//if CDR Loss lock count is greater than 0 show additional info
	if(stCaptureInfo.bTraceValid && stCaptureInfo.nCDRLossLockCount > 0)
	{
		SetDlgItemText(IDC_TRACE_VALID, "YES*");    

		sText.Format("*This trace may contain erroneous data due to Loss of CDR Lock (%d times).", stCaptureInfo.nCDRLossLockCount);
		SetDlgItemText(IDC_CDR_LOSS_LOCK, sText);
		GetDlgItem(IDC_CDR_LOSS_LOCK)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_CDR_LOSS_LOCK)->ShowWindow(SW_HIDE);
		sText.Format("%s", (stCaptureInfo.bTraceValid) ? "YES":"NO");
		SetDlgItemText(IDC_TRACE_VALID, sText);    
	}

	
	return;
    }
