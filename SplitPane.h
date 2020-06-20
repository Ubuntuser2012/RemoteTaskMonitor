#pragma once


// SplitPane

class SplitPane : public CView
{
	DECLARE_DYNCREATE(SplitPane)

public:
	SplitPane();
	virtual ~SplitPane();
	void OnDraw(CDC* pDC);

protected:
	DECLARE_MESSAGE_MAP()
};


