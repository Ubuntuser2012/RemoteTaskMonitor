#ifndef RAPHAEL_H
#define RAPHAEL_H
// Raphael.cpp : implementation file
//

class Raphael : public OBJECT
{
public:
	Raphael();
	virtual ~Raphael();

	bool draw(
      DEVICE *device, 
      STRING &outdir, 
      CDlgRemoteCPU* parent,
      LIST *history_tags,
      DATA_QUE &labels,
      DATA_QUE &cpu,
      DATA_QUE &mem);

private:
};

#endif//RAPHAEL_H