#include "stdafx.h"
#include <vector>

#include <gcroot.h>

#include "CDomainManagerWrapper.h"
#include "BookmarkManager.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace BookMark;

String^ char2String(const char* str)
{
    return gcnew System::String(str);
}

class CDomainManagerWrapperImpl
{
public:
    //ref class CaptureInfo
    //{
    //public:
    //	String^ sTraceDate;
    //	String^ sStopTime;
    //	String^ ProtocolAsString;
    //};

    gcroot<BookmarkManagerDlg^> pWpfWnd;
};

CDomainManagerWrapper::CDomainManagerWrapper(CDomainManager* pDomainMgr) : m_pDomainMgr(pDomainMgr)
{
    m_pImpl = new CDomainManagerWrapperImpl();

    BookmarkManagerDlg^ pDlg = gcnew BookmarkManagerDlg();
    m_pImpl->pWpfWnd = gcroot<BookmarkManagerDlg^>(pDlg);
}

CDomainManagerWrapper::~CDomainManagerWrapper()
{
    delete m_pImpl;
}

bool CDomainManagerWrapper::showWpfDlg()
{
    m_pImpl->pWpfWnd->CurrentDomainName = char2String(m_pDomainMgr->getCurrentDomainName());
    m_pImpl->pWpfWnd->NumberOfChannels = m_pDomainMgr->getNumberOfChannels();

    std::vector<CaptureInfo> arrCaptureInfos = m_pDomainMgr->getCaptureInfos();
    List<CaptureInfo_Wpf^>^ captureInfos = gcnew List<CaptureInfo_Wpf^>();

    for (int i = 0; i < arrCaptureInfos.size(); ++i)
    {
        auto _port = gcnew CaptureInfo_Wpf();
        _port->sTraceDate = char2String(arrCaptureInfos[i].sTraceDate);
        _port->sStopTime = char2String(arrCaptureInfos[i].sStopTime);
        _port->ProtocolAsString = char2String(arrCaptureInfos[i].ProtocolAsString);
        captureInfos->Add(_port);
    }

    m_pImpl->pWpfWnd->CaptureInfos = captureInfos;

    m_pImpl->pWpfWnd->ShowDialog();

    return true;
}

const char* CDomainManagerWrapper::getTraceInfoDlgInfo()
{
    System::String^ strDlgInfo = m_pImpl->pWpfWnd->getTraceInfoDlgInfo();

    return (const char*)Marshal::StringToHGlobalAnsi(strDlgInfo).ToPointer();
}

void CDomainManagerWrapper::WPFcallCPlus(int nCurrentIndex)
{
    ::AfxMessageBox(nCurrentIndex);
}

//
//gcroot<WpfApplication1::MainWindow^> CDomainManagerWrapper::InitTraceInfoDlg()
//{
//    return gcnew MainWindow();
//}

namespace Wrapper
{
    public ref class BookMarkMgr
    {
    public:
        BookMarkMgr()
        {
        };

        std::string Init();

        //System::String^ setCurrentSelect(int rowIndex);
    };
}

std::string Wrapper::BookMarkMgr::Init()
{
    CDomainManagerWrapper wrapper(new CDomainManager);
    wrapper.WPFcallCPlus(8);
    return "ss";
}

//System::String^ Wrapper::BookMarkMgr::setCurrentSelect(int rowIndex)
//{
//    return "888";
//}