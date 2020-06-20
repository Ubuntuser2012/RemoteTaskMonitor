#ifndef DIALOG2_H
#define DIALOG2_H

class CDialog2 : public CDialog
{
   bool m_closed;
public:
   CDialog2() : CDialog(), m_closed(false) {}
   void OnCancel();
   bool IsClosed() { return m_closed; }
};

#endif//DIALOG2_H