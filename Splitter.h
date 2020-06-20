#pragma once


// Splitter

class Splitter : public CSplitterWnd
{
	DECLARE_DYNAMIC(Splitter)

public:
	Splitter();
	virtual ~Splitter();
	void CreateSplitter(CWnd* parentWnd, CRuntimeClass* leftView, CRuntimeClass* rightView, const SIZE &sz, CCreateContext* pContext);
	void Size();
	void SetRowColInfo(int cx, int cy);
	void SetInitialized();

protected:
	bool m_initialized;
	int m_leftPaneWidthPercent;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


