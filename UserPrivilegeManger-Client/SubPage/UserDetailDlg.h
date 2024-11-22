#ifndef USERDETAILDLG_H_
#define USERDETAILDLG_H_

#include "PersonMessage.h"

class UserDetailDlg :public PersonMessage
{
public:
	UserDetailDlg(QWidget* parent = nullptr);
	void init();
private:
	QPushButton* m_backBtn;
};


#endif // !USERDETAILDLG_H_
