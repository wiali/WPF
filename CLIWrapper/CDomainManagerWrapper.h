#pragma once

#include "DomainManager.h"

class CDomainManagerWrapperImpl;

class __declspec(dllexport) CDomainManagerWrapper
{
public: 
	CDomainManagerWrapper(CDomainManager* pDomainMgr);
	~CDomainManagerWrapper();
    
	bool showWpfDlg();

	const char* getTraceInfoDlgInfo();

private:
	CDomainManagerWrapperImpl* m_pImpl;
	CDomainManager* m_pDomainMgr;
};
