#ifndef PERSONCENTERPAGE_H_
#define PERSONCENTERPAGE_H_

#include <QWidget>
#include "PersonMessage.h"

class PersonCenterPage :public PersonMessage
{
	Q_OBJECT
public:
	PersonCenterPage(QWidget* parent = nullptr);
	void init();
	void GetpersonMessage();
private:
	QPushButton* m_reLoginBtn{};
	QLabel* m_gradeLabel{};
public:
	void showUserGrade( const QString&userGrade);
signals:
	void reLogin();

};


#endif // !PERSONCENTERPAGE_H_
