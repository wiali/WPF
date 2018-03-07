#pragma once

#include <vector>

class CaptureInfo
{
public:
	char* sTraceDate;
	char* sStopTime;
	char* ProtocolAsString;
};

class __declspec(dllexport) CDomainManager
{
public:
	CDomainManager();
	virtual ~CDomainManager();

	char* getCurrentDomainName();
	int getNumberOfChannels();

	std::vector<CaptureInfo> getCaptureInfos();

private:
	std::vector<CaptureInfo> m_pCaptureInfo;
};

