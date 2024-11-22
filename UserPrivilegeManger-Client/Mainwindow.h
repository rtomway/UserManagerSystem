#ifndef MAINWINDOW_H_
#define  MAINWINDOW_H_

#include<QWidget>
#include<QStackedWidget>
#include"SNavBar.h"
#include<QTreeWidget>
#include <QPropertyAnimation>

#include "UserManagerPage.h"
#include "PManagerPage.h"
#include "PersonCenterPage.h"

class Mainwindow :public QWidget
{
	Q_OBJECT
public:
	Mainwindow(QWidget* parent = nullptr);
	void init();
private:
	SNavBar* m_navBar{};
	QStackedWidget* m_stkWidget{}; 

	QTreeWidget* m_treeNavBar;
	void initMainwindow();

	UserManagerPage* m_userManagerpage{};
	void initUserManagerPage();

	PManagerPage* m_pManagerpage{};
	void initPManagerPage();

	PersonCenterPage* m_personCenter{};
	void initPersonCenter();

	QPropertyAnimation* m_proAnimation{};
	QPropertyAnimation* m_MiddenAnimation{};
	QPropertyAnimation* m_RightAnimation{};
	bool m_treeIsHidden{false};
	QPushButton* m_hideBtn{};
	void updateBtnSize();

	int m_Grade=-1;
	
signals:
	void Personcenter(const QString &user_id);
	void reLogin();
public:
	void updateReloginlater();
	void GetUserGrade();
protected:
	void resizeEvent(QResizeEvent*ev)override;
};
#endif // !MAINWINDOW_H_
