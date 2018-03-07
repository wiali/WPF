#include "stdafx.h"
#include "DomainManager.h"


CDomainManager::CDomainManager()
{
	for (int i = 0; i < 3; ++i)
	{
		CaptureInfo pCaptureInfo;
		pCaptureInfo.sTraceDate = "20180307";
		pCaptureInfo.sStopTime = "20180308";
		pCaptureInfo.ProtocolAsString = "SAS/SATA";

		m_pCaptureInfo.push_back(pCaptureInfo);
	}
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