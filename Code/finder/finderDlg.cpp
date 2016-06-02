    
// finderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "finder.h"
#include "finderDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CfinderDlg dialog




CfinderDlg::CfinderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CfinderDlg::IDD, pParent), _fullDir(NULL), _fullFile(NULL), _upperDir(NULL), _upperFile(NULL)
{
	//m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CfinderDlg::~CfinderDlg()
{
    delete _fullDir;
    delete _fullFile;
    delete _upperDir;
    delete _upperFile;
}

void CfinderDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST2, _listCtrl);
    DDX_Control(pDX, IDC_EDIT_KEYWORD, _edit);
    DDX_Control(pDX, IDC_COMBO_TYPE, _typeCombo);
    DDX_Control(pDX, IDC_COMBO_NAME_SPACE, _nameSpaceCombo);
    DDX_Control(pDX, IDC_BUTTON_RELOAD, _reloadBtn);
    DDX_Control(pDX, IDC_BUTTON_OPEN_CONFIG, _openConfigBtn);
}

BEGIN_MESSAGE_MAP(CfinderDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CfinderDlg::OnBnClickedOk)
    ON_EN_CHANGE(IDC_EDIT_KEYWORD, &CfinderDlg::OnEnChangeEditKeyword)
    ON_WM_TIMER()
    ON_WM_SIZING()
    ON_WM_SIZE()
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST2, &CfinderDlg::OnCustomdrawList2)
    ON_BN_CLICKED(IDC_BUTTON_RELOAD, &CfinderDlg::OnBnClickedButtonReload)
    ON_WM_SHOWWINDOW()
    ON_WM_HELPINFO()
    ON_BN_CLICKED(IDC_BUTTON_OPEN_CONFIG, &CfinderDlg::OnBnClickedButtonOpenConfig)
    ON_WM_MOVE()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

static void MakeIconImageList(const CStringArray& exts, CImageList* plist)
{
    /*WORD dwIndex; 
    HICON hIcon = ExtractAssociatedIconW(GetModuleHandle(NULL), L"d:\\colorPalette.png", &dwIndex);
    static CImageList list;
    list.Add(hIcon);*/

    /*WORD dwIndex; 
    HICON hIcon = ExtractAssociatedIconW(GetModuleHandle(NULL), L"d:\\work", &dwIndex);*/
    
    
    PIDLIST_ABSOLUTE PIDL;
    SHGetSpecialFolderLocation(0, CSIDL_COMMON_DESKTOPDIRECTORY, &PIDL);

    SHFILEINFO sfi;
    memset(&sfi, 0, sizeof(sfi));
    SHGetFileInfo((LPCTSTR)PIDL, 0, &sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_PIDL);
    plist->Add(sfi.hIcon);

    for (int i = 0; i < exts.GetSize(); ++i) {
        memset(&sfi, 0, sizeof(sfi));
        SHGetFileInfo("*." + exts[i], 0, &sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES | SHGFI_ICON);
        plist->Add(sfi.hIcon);
    }
}

CBitmap* CfinderDlg::iconToBitmap(CWnd *pWnd,HICON hIcon) {
    CDC dcNew;
    CBitmap imageBitmap;
    ICONINFO iconInfo;
    HBITMAP hOldBitmap = NULL;
    BITMAP imageInfo;

    /*Get view CDC*/
    CDC* tmpDc = pWnd->GetWindowDC();

    CBitmap *pNewBitmap = new CBitmap();

    /*Get and construct colour and mask bitmaps*/
    GetIconInfo(hIcon, &iconInfo);
    imageBitmap.Attach(iconInfo.hbmColor);

    /*Get bitmap info*/
    imageBitmap.GetObject(sizeof(BITMAP), (LPSTR)&imageInfo);
    dcNew.CreateCompatibleDC(tmpDc);
    /*Create new compatible bitmap*/
    pNewBitmap->CreateCompatibleBitmap(tmpDc, imageInfo.bmWidth,
        imageInfo.bmHeight);

    if(!hIcon) return NULL;

    hOldBitmap = (HBITMAP)dcNew.SelectObject(pNewBitmap);
    COLORREF textColorSave = dcNew.SetTextColor (0x00000000L);
    COLORREF bkColorSave   = dcNew.SetBkColor (RGB(240, 240, 240));
    dcNew.FillSolidRect(0, 0, imageInfo.bmWidth,
        imageInfo.bmHeight,RGB(240, 240, 240));

    if(!DrawIconEx(dcNew, 0, 0, hIcon, imageInfo.bmWidth, imageInfo.bmHeight
        , NULL, NULL, DI_NORMAL))

        /*AfxMessageBox("Failed on DrawIcon"); */
        return NULL;

    /*dcNew.SetBkColor(colorOldBG);*/
    dcNew.SetTextColor (textColorSave);
    dcNew.SetBkColor (bkColorSave);
    pWnd->ReleaseDC(tmpDc);
    return pNewBitmap; 
}

// CfinderDlg message handlers
BOOL CfinderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    InitAll(false);
    
    SetTimer(1, 10, NULL);
    CRect& rt = _config.windowRect;
    SetWindowPos(NULL, rt.left, rt.top, rt.Width(), rt.Height(), SWP_NOZORDER);
    ChangeStatus(VS_MINIMIZE);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CfinderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CfinderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CfinderDlg::OnBnClickedOk()
{
    
    //CDialogEx::OnOK();
}


void CfinderDlg::OnEnChangeEditKeyword()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialogEx::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
    CString str;
    GetDlgItemText(IDC_EDIT_KEYWORD, str);
    UpdateKeyword((LPCTSTR)str);
}

static void LoadCStringArrayFromINI(LPCTSTR file, LPCTSTR app, LPCTSTR key, LPCTSTR defaults, LPCTSTR separator, CStringArray* stringArray)
{
    char buffer[2048];
    GetPrivateProfileString(app, key, defaults, buffer, dimof(buffer), file);
    if (GetLastError() == 2) {
        WritePrivateProfileString(app, key, defaults, file);
    }

    int index = 0;
    CString strBuf = buffer;
    stringArray->RemoveAll();

    while (index < strBuf.GetLength()) {
        stringArray->Add(strBuf.Tokenize(separator, index));
    }
}

void CfinderDlg::LoadConfig(LPCTSTR file)
{
    // path
    const char* defaultPath = "d:\\filedownload;d:\\putty;d:\\work";
    LoadCStringArrayFromINI(file, "Include", "path", defaultPath, ";", &(_config.includePaths));

    // extention
    const char* defaultExt = "exe;bat;cfg;ini;pdb;dll;log,dmp;zip;cpp;h;hpp;lib;rb;php;sqlite;xml;lua;g";
    LoadCStringArrayFromINI(file, "Include", "ext", defaultExt, ";", &(_config.includeExts));

    // exclude path
    const char* defaultExcludePath = "";
    LoadCStringArrayFromINI(file, "Exclude", "path", defaultExcludePath, ";", &(_config.excludePaths));

    // etc programs
    const char* defaultProgram = "E:\\filedownload\\npp.6.4.2.bin.minimalist\\notepad++.exe;"
                                 "C:\\Program Files (x86)\\WinMerge\\WinMergeU.exe";
    LoadCStringArrayFromINI(file, "Etc", "program", defaultProgram, ";", &(_config.etcPrograms));


    char buffer[2048];
    const char* defaultObject = "all";
    GetPrivateProfileString("Option", "search_object", defaultObject, buffer, dimof(buffer), file);
    if (GetLastError() == 2) {
        WritePrivateProfileString("Option", "search_object", defaultObject, file);
    }
    _config.objectType = buffer;

    const char* defaultType = "name";
    GetPrivateProfileString("Option", "search_type", defaultType, buffer, dimof(buffer), file);
    if (GetLastError() == 2) {
        WritePrivateProfileString("Option", "search_type", defaultType, file);
    }
    _config.searcheType = buffer;


    CString str;
    UINT width = GetPrivateProfileInt("Window", "width", 900, file);
    if (GetLastError() == 2) {
        WritePrivateProfileString("Window", "width", "900", file);
    }

    UINT height = GetPrivateProfileInt("Window", "height", 400, file);
    if (GetLastError() == 2) {
        WritePrivateProfileString("Window", "height", "400", file);
    }

    UINT x = GetPrivateProfileInt("Window", "x", 0, file);
    if (GetLastError() == 2) {
        x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
        str.Format("%d", x);
        WritePrivateProfileString("Window", "x", str, file);
    }

    UINT y = GetPrivateProfileInt("Window", "y", INT_MAX, file);
    if (GetLastError() == 2) {
        y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
        str.Format("%d", y);
        WritePrivateProfileString("Window", "y", str, file);
    }

    _config.windowRect.left = x;
    _config.windowRect.top = y;
    _config.windowRect.right = x + width;
    _config.windowRect.bottom = y + height;
}

void CfinderDlg::SaveConfig(LPCTSTR file, UINT configMask)
{
    if (configMask & Config::WINDOW) {
        char buf[255];
        CRect& rt = _config.windowRect;

        wsprintf(buf, "%d", rt.Width());
        WritePrivateProfileString("Window", "width", buf, file);

        wsprintf(buf, "%d", rt.Height());
        WritePrivateProfileString("Window", "height", buf, file);

        wsprintf(buf, "%d", rt.left);
        WritePrivateProfileString("Window", "x", buf, file);

        wsprintf(buf, "%d", rt.top);
        WritePrivateProfileString("Window", "y", buf, file);
    }
}

#define SAFE_DELETE(ptr) \
    if (ptr) { delete ptr; ptr = NULL; }

void CfinderDlg::InitAll(bool forceReload)
{
    char buf[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buf);
    CString curPath = buf;

    LoadConfig(curPath + "\\finder.ini");

    SAFE_DELETE(_fullDir);
    SAFE_DELETE(_fullFile);

    _fullDir = new IntegratedFileObject();
    _fullFile = new IntegratedFileObject();

    int count = 0;
    bool loaded = false;
    if (!forceReload) {
        loaded = _fullDir->Load(curPath + "\\finder_dirs.dat") && _fullFile->Load(curPath + "\\finder_files.dat");
    }

    if (!loaded) {
        count = MakeFileObject(_config.includePaths, _config.includeExts, _config.excludePaths, _fullFile, _fullDir);
        _fullDir->Save(curPath + "\\finder_dirs.dat");
        _fullFile->Save(curPath + "\\finder_files.dat");
    }

    SAFE_DELETE(_upperDir);
    SAFE_DELETE(_upperFile);

    _upperDir = _fullDir->Clone();
    _upperFile = _fullFile->Clone();

    IntegratedFileObject* work;

    class Upper {
    public:
        void operator () (LPSTR name) { strupr(name); }
    };

    work = _upperDir;
    work->Rounds(Upper());
    work = _upperFile;
    work->Rounds(Upper());

    CString temp;
    temp.Format("my finder %d loaded", count);
    SetWindowText(temp);

    CImageList* imageList = new CImageList;
    imageList->Create(24,24,ILC_COLOR32,0,0);
    MakeIconImageList(_config.includeExts, imageList);

    _listCtrl.SetImageList(imageList, LVSIL_SMALL);
    _listCtrl.InsertColumn(0, "name", LVCFMT_LEFT, 100);
    _listCtrl.InsertColumn(1, "path", LVCFMT_LEFT, 500);
    _listCtrl.SetExtendedStyle(_listCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    _typeCombo.InsertString(0, "All");
    _typeCombo.SetItemData(0, FileObject::TYPE_NONE);
    _typeCombo.SetCurSel(0);

    _typeCombo.InsertString(1, "File");
    _typeCombo.SetItemData(1, FileObject::TYPE_FILE);
    if (_config.objectType.CompareNoCase("file")) {
        _nameSpaceCombo.SetCurSel(1);
    }

    _typeCombo.InsertString(2, "Dir");
    _typeCombo.SetItemData(2, FileObject::TYPE_DIR);
    if (_config.objectType.CompareNoCase("dir")) {
        _nameSpaceCombo.SetCurSel(2);
    }

    _nameSpaceCombo.InsertString(0, "Full");
    _nameSpaceCombo.SetItemData(0, 0);
    _nameSpaceCombo.InsertString(1, "Name");
    _nameSpaceCombo.SetItemData(1, 1);
    _nameSpaceCombo.SetCurSel(0);

    _modified = FALSE;

    _ctrlMenu.CreatePopupMenu();
    _altMenu.CreatePopupMenu();

    for (int i = 0; i < _config.etcPrograms.GetSize(); ++i) {
        int menuIndex = 20000 + i;
        LPCTSTR name = PathFindFileName(_config.etcPrograms[i]);
        _ctrlMenu.AppendMenu(MF_STRING, menuIndex, name);
        _altMenu.AppendMenu(MF_STRING, menuIndex + 10000, name);

        char buf2[1024];
        _tcscpy(buf2, (LPCTSTR)_config.etcPrograms[i]);

        WORD dwIndex;
        HICON hIcon = ExtractAssociatedIcon(GetModuleHandle(NULL), buf2, &dwIndex);
        CBitmap* bitmap = iconToBitmap(this, hIcon);

        _ctrlMenu.SetMenuItemBitmaps(menuIndex, MF_BYCOMMAND, bitmap, bitmap);
        _altMenu.SetMenuItemBitmaps(menuIndex + 10000, MF_BYCOMMAND, bitmap, bitmap);
    }
}

void CfinderDlg::UpdateKeyword(LPCTSTR str)
{
    _lastKeyword = str;
     QueryPerformanceCounter(&_lastKeywordUpdated);
     _modified = TRUE;
}


class FileObjectFiller
{
public:
    FileObjectFiller(CListCtrl* listCtrl, CStringArray* exts, LPCTSTR str, FileObject::ObjectType searchType, BOOL searchFromFullPath) 
    : _listCtrl(listCtrl), _exts(exts), _str(str), _searchType(searchType), _searchFromFullPath(searchFromFullPath) {}

    void operator() (FileObject* object)
    {
        if (!(object->_type & _searchType)) {
            return;
        }

        int index = _listCtrl->GetItemCount();
        if (index > 500) {
            return;
        }


        LPCTSTR dst = _searchFromFullPath ? object->_fullName_i : object->_fileName_i;
        if (!strstr(dst, _str)) {
            return;
        }

        int imageIndex = 0;
        if (object->_type == FileObject::TYPE_FILE) {
            for (int i = 0; i < _exts->GetSize(); ++i) {
                if (_exts->GetAt(i).CompareNoCase(PathFindExtension(object->_fileName) + 1) == 0) {
                    imageIndex = i + 1;
                    break;
                }
            }
        }

        _listCtrl->InsertItem(index, object->_fileName, imageIndex);
        _listCtrl->SetItemText(index, 1, object->_fullName);
        _listCtrl->SetItemData(index, (DWORD_PTR)object);
    }

private:
    CListCtrl* _listCtrl;
    CStringArray* _exts;
    LPCTSTR _str;
    FileObject::ObjectType _searchType;
    BOOL _searchFromFullPath;
};


static void MakeWindowStylePath(char* path)
{
    while(*path) {
        if (*path == '/') {
            *path = '\\';
        }

        ++path;
    }
}

void CfinderDlg::UpdateList(LPCTSTR str)
{
    _listCtrl.LockWindowUpdate();
    _listCtrl.DeleteAllItems();
    
    if (strlen(str) > 0) {
        char buf[1024];
        _tcscpy(buf, str);
        strupr(buf);
        MakeWindowStylePath(buf);

        _listCtrl.ShowWindow(SW_HIDE);

        FileObject::ObjectType type = (FileObject::ObjectType)_typeCombo.GetItemData(_typeCombo.GetCurSel());
        BOOL full = (int)_nameSpaceCombo.GetItemData(_nameSpaceCombo.GetCurSel()) == 0;

        FileObjectFiller filler(&_listCtrl, &_config.includeExts, buf, type, full);

        {
            CDWordArray offsets;
            _upperDir->FindOffsets(buf, &offsets);

            CStringArray arr;
            _fullDir->NameFromOffsets(offsets, &arr);

            int insertIndex = _listCtrl.GetItemCount();
            int count = arr.GetSize();
            for (int index = 0; index < count; ++index) {
                const CString& name = arr.GetAt(index);

                TCHAR basePath[MAX_PATH];
                _tcscpy_s(basePath, MAX_PATH, (LPCTSTR)name);
                PathRemoveFileSpec(basePath);

                _listCtrl.InsertItem(insertIndex, PathFindFileName(name), 0);
                _listCtrl.SetItemText(insertIndex, 1, basePath);
                ++insertIndex;
            }
        }

        {
            CDWordArray offsets;
            _upperFile->FindOffsets(buf, &offsets);

            CStringArray arr;
            _fullFile->NameFromOffsets(offsets, &arr);

            int insertIndex = _listCtrl.GetItemCount();
            int count = arr.GetSize();
            for (int index = 0; index < count; ++index) {
                const CString& name = arr.GetAt(index);

                int imageIndex = 0;
                for (int i = 0; i < _config.includeExts.GetSize(); ++i) {
                    if (_config.includeExts.GetAt(i).CompareNoCase(PathFindExtension(name) + 1) == 0) {
                        imageIndex = i + 1;
                        break;
                    }
                }

                TCHAR basePath[MAX_PATH];
                _tcscpy_s(basePath, MAX_PATH, (LPCTSTR)name);
                PathRemoveFileSpec(basePath);
                
                _listCtrl.InsertItem(insertIndex, PathFindFileName(name), imageIndex);
                _listCtrl.SetItemText(insertIndex, 1, basePath);
                ++insertIndex;
                //_listCtrl->SetItemData(index, (DWORD_PTR)object);
            }
        }

        //_listCtrl.SetItemState(0, LVIS_SELECTED, 0x000F);
        _listCtrl.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
        _listCtrl.ShowWindow(SW_SHOW);    
    }

    _listCtrl.UnlockWindowUpdate();
}


void CfinderDlg::UpdateView(ViewStatus viewStatus)
{
    _reloadBtn.ShowWindow(SW_HIDE);
    _openConfigBtn.ShowWindow(SW_HIDE);


    if (viewStatus == VS_SEARCH) {
        _listCtrl.ShowWindow(SW_SHOW);
        _typeCombo.ShowWindow(SW_SHOW);
        _nameSpaceCombo.ShowWindow(SW_SHOW);
        _edit.ShowWindow(SW_SHOW);
        _reloadBtn.ShowWindow(SW_HIDE);
        _openConfigBtn.ShowWindow(SW_HIDE);
        _edit.SetFocus();

    } else if (viewStatus == VS_OPTION) {
        _listCtrl.ShowWindow(SW_HIDE);
        _typeCombo.ShowWindow(SW_HIDE);
        _nameSpaceCombo.ShowWindow(SW_HIDE);
        _edit.ShowWindow(SW_HIDE);
        _reloadBtn.ShowWindow(SW_SHOW);
        _openConfigBtn.ShowWindow(SW_SHOW);
    }
}

void CfinderDlg::ChangeStatus(ViewStatus viewStatus)
{
    if (viewStatus == VS_SEARCH) {
        if (IsIconic()) {
            ShowWindow(SW_RESTORE);
        } else {
            BringWindowToTop();
            HWND hwndForeground = GetForegroundWindow()->GetSafeHwnd();
            DWORD idFGThread = GetWindowThreadProcessId (hwndForeground, NULL);
            DWORD idThisThread = GetCurrentThreadId();
            //VERIFY (AttachThreadInput (idThisThread, idFGThread, TRUE));
            SetForegroundWindow();
        }
        
    } else if (viewStatus == VS_OPTION) {
        ShowWindow(SW_RESTORE);
    } if (viewStatus == VS_MINIMIZE) {
        ShowWindow(SW_MINIMIZE);
    }

    _viewStatus = viewStatus;
    UpdateView(viewStatus);
}


static BOOL IsOpenKeyPressed()
{
    const int keyList[] = {
        VK_SHIFT,
        VK_LMENU,
        'I',
    };
    
    for (int i = 0; i < dimof(keyList); ++i) {
        if (!(GetAsyncKeyState(keyList[i]) & 0x8000)) {
            return FALSE;
        }
    }

    return TRUE;
}

void CfinderDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (IsOpenKeyPressed()) {
        ChangeStatus(VS_SEARCH);
        return;
    }

    // TODO: Add your message handler code here and/or call default
    if (!_modified) {
        return;
    }

    LARGE_INTEGER current, preq;
    QueryPerformanceCounter(&current);
    QueryPerformanceFrequency(&preq);

    long double time = (current.QuadPart - _lastKeywordUpdated.QuadPart) / (long double)preq.QuadPart;
    time *= 1000.0;

    if (time > 100.0) {
        UpdateList((LPCTSTR)_lastKeyword);
        _modified = FALSE;
    }

    CDialogEx::OnTimer(nIDEvent);
}

BOOL CfinderDlg::HandleForwardToListCtrl(MSG* pMsg)
{
    if (pMsg->hwnd != _edit.GetSafeHwnd()) {
        return FALSE;
    }

    if (pMsg->message != WM_KEYDOWN && pMsg->message == WM_KEYUP) {
        return FALSE;
    }

    int upDownKeys[] = {
        VK_UP,
        VK_DOWN,
        VK_PRIOR,
        VK_NEXT,
//        VK_HOME,
//        VK_END,
    };

    int key = (int)pMsg->wParam;
    BOOL findKey = FALSE;
    int size = sizeof(upDownKeys) / sizeof(upDownKeys[0]);

    for (int i = 0; i < size; ++i) {
        if (upDownKeys[i] == key) {
            findKey = TRUE;
            break;
        }
    }

    if (!findKey) {
        findKey = (pMsg->wParam == VK_SPACE && (GetAsyncKeyState(VK_CONTROL) & 0x8000));
    }

    if (!findKey) {
        return FALSE;
    }

    _listCtrl.SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
    return TRUE;
}

BOOL CfinderDlg::HandleEnter(MSG* pMsg)
{
    if (pMsg->hwnd != _edit.GetSafeHwnd() && pMsg->hwnd != _listCtrl.GetSafeHwnd()) {
        return FALSE;
    }

    if (pMsg->message != WM_KEYDOWN && pMsg->message == WM_SYSKEYDOWN) {
        return FALSE;
    }

    int key = (int)pMsg->wParam;
    if (key != VK_RETURN) {
        return FALSE;
    }

    CRect rt;
    GetWindowRect(rt);
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
        _ctrlMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rt.right, rt.top, this);
        return FALSE;

    } else if (GetAsyncKeyState(VK_LMENU) & 0x8000) {
        _altMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rt.right, rt.top, this);
        return FALSE;

    } else {
        POSITION pos = _listCtrl.GetFirstSelectedItemPosition();
        while (pos) {
            int itemIndex = _listCtrl.GetNextSelectedItem(pos);
            //FileObject* object = (FileObject*)_listCtrl.GetItemData(itemIndex);
            //LPCTSTR fullName = _listCtrl.GetItemData(itemIndex);
            CString basePath = _listCtrl.GetItemText(itemIndex, 1);
            CString fullName = basePath + "\\";
            fullName += _listCtrl.GetItemText(itemIndex, 0);
            ShellExecute(NULL, NULL, fullName, NULL, basePath, SW_SHOW);
        }
    }

    ChangeStatus(VS_MINIMIZE);
    
    return TRUE;
}

BOOL CfinderDlg::HandleEscape(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
        if (_viewStatus == VS_OPTION) {
            ChangeStatus(VS_SEARCH);
        } else {
            ChangeStatus(VS_MINIMIZE);
        }
        
        return TRUE;
    }

    return FALSE;
}

BOOL CfinderDlg::HandleExitKey(MSG* pMsg)
{
    return WM_SYSKEYDOWN == pMsg->message && VK_F4 == pMsg->wParam;
}

static void RoundComboIndex(CComboBox& box)
{
    int index = box.GetCurSel() + 1;
    int count = box.GetCount();
    
    if (count == 0) {
        return;
    }

    if (count == index) {
        index = 0;
    } 

    box.SetCurSel(index);
}

BOOL CfinderDlg::HandleOptionChangeKey(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F8) {
        RoundComboIndex(_typeCombo);
        return TRUE;
    }

    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F9) {
        RoundComboIndex(_nameSpaceCombo);
        return TRUE;
    }

    return FALSE;
}

BOOL CfinderDlg::HandleEventForFunctionality(MSG* pMsg)
{
    if (HandleForwardToListCtrl(pMsg)) {
        return TRUE;
    }

    if (HandleEnter(pMsg)) {
        return TRUE;
    }

    if (HandleEscape(pMsg)) {
        return TRUE;
    }

    if (HandleExitKey(pMsg)) {
        return TRUE;
    }

    if (HandleOptionChangeKey(pMsg)) {
        return TRUE;
    }

    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F1) {
        ChangeStatus(VS_OPTION);
        return TRUE;
    }

    return FALSE;
}

BOOL CfinderDlg::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class
    if (HandleEventForFunctionality(pMsg)) {
        return TRUE;
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}

void CfinderDlg::ResetChildWindowSize()
{
    if (IsWindow(_listCtrl.GetSafeHwnd()) && IsWindow(_edit.GetSafeHwnd())) {
        CRect rt;
        GetClientRect(&rt);

        int val = 10;
        CRect inset;
        inset.left = val;
        inset.top = val;
        inset.right = val * 2;
        inset.bottom = val * 2;

        _listCtrl.MoveWindow(inset.left, inset.top, rt.Width() - inset.right, rt.Height() - (inset.bottom + 30), TRUE);
        _edit.MoveWindow(inset.left + 105, rt.Height() - 35, rt.Width() - (inset.right + 105), 25, TRUE);
        _typeCombo.SetWindowPos(NULL, inset.left, rt.Height() - 35, 0, 0, SWP_NOSIZE);
        _nameSpaceCombo.SetWindowPos(NULL, inset.left + 50, rt.Height() - 35, 0, 0, SWP_NOSIZE);
    }
}

void CfinderDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
    CDialogEx::OnSizing(fwSide, pRect);

    // TODO: Add your message handler code here
    ResetChildWindowSize();
}


void CfinderDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    ResetChildWindowSize();
    if (nType == 0) {
        GetWindowRect(_config.windowRect);
    }
}

void CfinderDlg::OnMove(int x, int y)
{
    CDialogEx::OnMove(x, y);

    if (x != -32000) {
        GetWindowRect(_config.windowRect);
    }
}

void CfinderDlg::OnCustomdrawList2(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVCUSTOMDRAW * pCustom = (NMLVCUSTOMDRAW *)pNMHDR;

    switch(pCustom->nmcd.dwDrawStage) {
    case CDDS_PREPAINT :
        *pResult = CDRF_NOTIFYITEMDRAW;
        return;
    case CDDS_ITEMPREPAINT:
        
        if (_listCtrl.GetItemState(pCustom->nmcd.dwItemSpec, LVIS_SELECTED)) {
            pCustom->clrTextBk = RGB(162,189,249);
            pCustom->nmcd.uItemState &= ~CDIS_SELECTED;
        } else if (_listCtrl.GetItemState(pCustom->nmcd.dwItemSpec, LVIS_FOCUSED)) {
            pCustom->clrTextBk = RGB(249, 189,162);
            pCustom->nmcd.uItemState &= ~CDIS_FOCUS;
        }
        *pResult = 0;
        return;
    default:
        *pResult = 0;
        return;
    } 
}


BOOL CfinderDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // TODO: Add your specialized code here and/or call the base class
    int menuId = LOWORD(wParam);

    if (menuId < 31000) {
        if (menuId >= 30000) {
            int index = menuId - 30000;

            CString str;
            CString fullName;
            POSITION pos = _listCtrl.GetFirstSelectedItemPosition();
            while (pos) {
                int itemIndex = _listCtrl.GetNextSelectedItem(pos);
                fullName = _listCtrl.GetItemText(itemIndex, 1);
                fullName += '\\';
                fullName += _listCtrl.GetItemText(itemIndex, 0);

                str += "\"";
                str += fullName;
                str += "\" ";
            }

            str.Replace("\r", "");
            str.Replace("\n", "");
            ShellExecute(NULL, NULL, "\"" + _config.etcPrograms[index] + "\"", str, NULL, SW_SHOW);
            ChangeStatus(VS_MINIMIZE);
        } else if (menuId >= 20000) {
            int index = menuId - 20000;

            POSITION pos = _listCtrl.GetFirstSelectedItemPosition();
            while (pos) {
                int itemIndex = _listCtrl.GetNextSelectedItem(pos);
                CString fullName = _listCtrl.GetItemText(itemIndex, 1);
                fullName += '\\';
                fullName += _listCtrl.GetItemText(itemIndex, 0);

                ShellExecute(NULL, NULL, "\"" + _config.etcPrograms[index] + "\"", fullName, NULL, SW_SHOW);
            }

            ChangeStatus(VS_MINIMIZE);
        }
    }
    
    return CDialogEx::OnCommand(wParam, lParam);
}


void CfinderDlg::OnBnClickedButtonReload()
{
    // TODO: Add your control notification handler code here
    UpdateList("");
    _edit.SetWindowText("");

    InitAll(true);

    /*delete _rootObj;
    _rootObj = new FileObject;*/
    
    //LoadConfig(curPath + "\\finder.ini");

    //int count = MakeTree(_config.includePaths, _config.includeExts, _rootObj);
    //if (count) {
    //    SaveTree(curPath + "\\finder.dat", _rootObj);
    //}
}


void CfinderDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialogEx::OnShowWindow(bShow, nStatus);
    _edit.SetFocus();
}


BOOL CfinderDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return TRUE;
}


void CfinderDlg::OnBnClickedButtonOpenConfig()
{
    // TODO: Add your control notification handler code here
    char buf[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buf);
    CString curPath = buf;
    ShellExecute(NULL, NULL, curPath + "\\finder.ini", NULL, NULL, SW_SHOW);
}

void CfinderDlg::OnClose()
{
    // TODO: Add your message handler code here and/or call default
    char buf[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buf);
    CString curPath = buf;
    SaveConfig(curPath + "\\finder.ini", Config::WINDOW);

    CDialogEx::OnClose();
}
