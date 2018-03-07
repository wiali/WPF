#include "stdafx.h"
#include <vector>

#include <msclr\auto_gcroot.h>

#include "CDomainManagerWrapper.h"
#include "DomainManager.h"


using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices; // Marshal
using namespace WpfApplication1;

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

	gcroot<MainWindow^> pWpfWnd;
};

CDomainManagerWrapper::CDomainManagerWrapper(CDomainManager* pDomainMgr) : m_pDomainMgr(pDomainMgr)
{
	m_pImpl = new CDomainManagerWrapperImpl();
	m_pImpl->pWpfWnd = gcnew MainWindow();
}

CDomainManagerWrapper::~CDomainManagerWrapper()
{
	delete m_pImpl;
}

bool CDomainManagerWrapper::showWpfDlg()
{
	m_pImpl->pWpfWnd->setCurrentDomainName(char2String(m_pDomainMgr->getCurrentDomainName()));
	m_pImpl->pWpfWnd->setNumberOfChannels(m_pDomainMgr->getNumberOfChannels());

	std::vector<CaptureInfo> arrCaptureInfos = m_pDomainMgr->getCaptureInfos();
	cli::array<CaptureInfo_Wpf^>^ captureInfos =
		gcnew cli::array<CaptureInfo_Wpf^>(arrCaptureInfos.size());

	for (int i = 0; i < arrCaptureInfos.size(); ++i)
	{
		captureInfos[i] = gcnew CaptureInfo_Wpf();
		captureInfos[i]->sTraceDate = char2String(arrCaptureInfos[i].sTraceDate);
		captureInfos[i]->sStopTime = char2String(arrCaptureInfos[i].sStopTime);
		captureInfos[i]->ProtocolAsString = char2String(arrCaptureInfos[i].ProtocolAsString);
	}

	m_pImpl->pWpfWnd->setCaptureInfos(captureInfos);

	m_pImpl->pWpfWnd->ShowDialog();

	return true;
}

const char* CDomainManagerWrapper::getTraceInfoDlgInfo()
{
	System::String^ strDlgInfo = m_pImpl->pWpfWnd->getTraceInfoDlgInfo();

	return (const char*)Marshal::StringToHGlobalAnsi(strDlgInfo).ToPointer();
}
