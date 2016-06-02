
// finderDlg.h : header file
//

#pragma once

#include "FileObject.h"
#include "afxcmn.h"
#include "customedit.h"
#include "afxwin.h"

enum ViewStatus 
{
    VS_SEARCH = 0,
    VS_OPTION,
    VS_MINIMIZE,
};

struct Config
{
    CRect windowRect;
    CStringArray includePaths;
    CStringArray includeExts;
    CStringArray etcPrograms;
    CStringArray excludePaths;

    CString objectType;
    CString searcheType;

static const UINT PATH = 1 << 0;
static const UINT EXT = 1 << 1;
static const UINT PROGRAM = 1 << 2;
static const UINT WINDOW = 1 << 3;
};


// CfinderDlg dialog
class CfinderDlg : public CDialogEx
{
// Construction
public:
	CfinderDlg(CWnd* pParent = NULL);	// standard constructor
    ~CfinderDlg();

// Dialog Data
	enum { IDD = IDD_FINDER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnEnChangeEditKeyword();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnCustomdrawList2(NMHDR *pNMHDR, LRESULT *pResult);

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    void ResetChildWindowSize();

    void UpdateList(LPCTSTR str);
    void UpdateKeyword(LPCTSTR str);
    void UpdateView(ViewStatus viewStatus);
    void ChangeStatus(ViewStatus viewStatus);

private:
    void LoadConfig(LPCTSTR file);
    void SaveConfig(LPCTSTR file, UINT configMask);
    void InitAll(bool forceReload);

    CBitmap* iconToBitmap(CWnd *pWnd,HICON hIcon);

    BOOL HandleForwardToListCtrl(MSG* pMsg);
    BOOL HandleEnter(MSG* pMsg);
    BOOL HandleEscape(MSG* pMsg);
    BOOL HandleExitKey(MSG* pMsg);
    BOOL HandleOptionChangeKey(MSG* pMsg);
    BOOL HandleEventForFunctionality(MSG* pMsg);

private:
    IntegratedFileObject* _fullFile;
    IntegratedFileObject* _fullDir;
    IntegratedFileObject* _upperFile;
    IntegratedFileObject* _upperDir;

    CString _lastKeyword;
    LARGE_INTEGER _lastKeywordUpdated;
    BOOL _modified;
    Config _config;
    ViewStatus _viewStatus;

public:
    CListCtrl _listCtrl;
    CCustomEdit _edit;
    CComboBox _typeCombo;
    CComboBox _nameSpaceCombo;
    CMenu _ctrlMenu;
    CMenu _altMenu;
    CButton _reloadBtn;

public:
    afx_msg void OnBnClickedButtonReload();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnBnClickedButtonOpenConfig();
    CButton _openConfigBtn;
    afx_msg void OnMove(int x, int y);
    afx_msg void OnClose();
};

