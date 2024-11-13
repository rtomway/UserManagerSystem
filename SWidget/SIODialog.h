#ifndef SDIODIALOG_H_
#define SDIODIALOG_H_

#include<QWidget>


class SIODialog :public QWidget
{
	Q_OBJECT
public:
	SIODialog(QWidget* parent=nullptr);
	void init();
 
};

#endif // !SDIODIALOG_H_
