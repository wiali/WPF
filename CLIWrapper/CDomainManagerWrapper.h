#pragma once

class BookmarkManager;
class CDomainManagerWrapperImpl;

class __declspec(dllexport) CDomainManagerWrapper
{
public:
    CDomainManagerWrapper(BookmarkManager* pDomainMgr);
    ~CDomainManagerWrapper();

    bool showWpfDlg();

    const char* getTraceInfoDlgInfo();

    void WPFcallCPlus(int nCurrentIndex);

private:
    CDomainManagerWrapperImpl* m_pImpl;
    BookmarkManager* m_pBookmarkManageMgr;
};


