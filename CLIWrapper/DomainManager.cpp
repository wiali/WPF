#include "stdafx.h"
#include "DomainManager.h"


CDomainManager::CDomainManager()
{
    CaptureInfo pCaptureInfo;
    pCaptureInfo.sTraceDate = "20160307";
    pCaptureInfo.sStopTime = "20160308";
    pCaptureInfo.ProtocolAsString = "SAS Port(1,1,1)";
    m_pCaptureInfo.push_back(pCaptureInfo);

    pCaptureInfo.sTraceDate = "20170307";
    pCaptureInfo.sStopTime = "20170308";
    pCaptureInfo.ProtocolAsString = "SATA Port(2,2,2)";
    m_pCaptureInfo.push_back(pCaptureInfo);

    pCaptureInfo.sTraceDate = "20180307";
    pCaptureInfo.sStopTime = "20180308";
    pCaptureInfo.ProtocolAsString = "PCIE Port(3,3,3)";
    m_pCaptureInfo.push_back(pCaptureInfo);
}

CDomainManager::~CDomainManager()
{
}

char* CDomainManager::getCurrentDomainName()
{
	return "Single-ACHI-SATA";
}

int CDomainManager::getNumberOfChannels()
{
	return 8;
}

std::vector<CaptureInfo> CDomainManager::getCaptureInfos()
{
	return m_pCaptureInfo;
}