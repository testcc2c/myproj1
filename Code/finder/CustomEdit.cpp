// CustomEdit.cpp : implementation file
//

#include "stdafx.h"
#include "finder.h"
#include "CustomEdit.h"


// CCustomEdit

IMPLEMENT_DYNAMIC(CCustomEdit, CEdit)

CCustomEdit::CCustomEdit()
{

}

CCustomEdit::~CCustomEdit()
{
}


BEGIN_MESSAGE_MAP(CCustomEdit, CEdit)
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
END_MESSAGE_MAP()



// CCustomEdit message handlers




void CCustomEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // TODO: Add your message handler code here and/or call default
    
    CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCustomEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // TODO: Add your message handler code here and/or call default

    CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}
