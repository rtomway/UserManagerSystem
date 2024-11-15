#ifndef PERSONMESSAGE_H_
#define PERSONMESSAGE_H_

#include <QWidget>
#include <QJsonObject>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include "SSwitchButton.h"
#include "UsereditDlg.h"

class PersonMessage :public QWidget
{
	Q_OBJECT
public:
	PersonMessage(QWidget* parent = nullptr);
	void init();
	void setUser(const QJsonObject& user);
private:
	QWidget* m_leftLayout{};
public:
	QWidget* leftLayout()const;
signals:
	void userChanged(const QJsonObject&user);
protected://slots
	void onAvatarDownload();
	void onAvatarUpload();
private:
	QJsonObject m_juser;
	QPushButton* m_backBtn{};
	QLabel* m_avatar_lab{};
	QLabel* m_user_id_lab{};
	QLabel* m_username_lab{};
	QLabel* m_mobile_lab{};
	QLabel* m_email_lab{};
	QLabel* m_gender_lab{};
	SSwitchButton* m_isEnable_btn{};
signals:
	void setshow();
	void userDeleted(QJsonObject&user);
private:
	UsereditDlg* m_userEditDlg{};
};

#endif // !PERSONMESSAGE_H_
