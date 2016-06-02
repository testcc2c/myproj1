#pragma once


// CCustomEdit

class CCustomEdit : public CEdit
{
	DECLARE_DYNAMIC(CCustomEdit)

public:
	CCustomEdit();
	virtual ~CCustomEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};


