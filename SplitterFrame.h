#pragma once

#include "Splitter.h"

// SplitterFrame frame

class SplitterFrame : public CFrameWnd
{
	DECLARE_DYNAMIC(SplitterFrame)
public:
	SplitterFrame(CRuntimeClass* leftView, CRuntimeClass* rightView);           // protected constructor used by dynamic creation
	virtual ~SplitterFrame();
	const Splitter &GetSplitterWnd();
	void Size();

protected:
	bool m_initialized;
	Splitter m_splitter;
	CRuntimeClass* m_leftView;
	CRuntimeClass* m_rightView;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


