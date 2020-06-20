#ifndef CPROPERTYPAGEBASE_H
#define CPROPERTYPAGEBASE_H

class CDlgRemoteCPU;

class PropPageBase : public CPropertyPage
{
public:
	PropPageBase() :
		m_input_handler(0), CPropertyPage() {}
	PropPageBase(UINT nIDTemplate, UINT nIDCaption = 0) :
		CPropertyPage(nIDTemplate, nIDCaption) {}
	PropPageBase(LPCTSTR lpszTemplateName, UINT nIDCaption = 0) :
		CPropertyPage(lpszTemplateName, nIDCaption) {}
	virtual ~PropPageBase() {}

	void SetUserInputHandler(CDlgRemoteCPU *handler);

protected:
	CDlgRemoteCPU *m_input_handler;

};

#endif//CPROPERTYPAGEBASE_H

