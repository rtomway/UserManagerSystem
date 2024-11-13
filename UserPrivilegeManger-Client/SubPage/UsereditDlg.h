#ifndef USEREDITDLG_H_
#define USEREDITDLG_H_

#include <QWidget>
#include <QJsonObject>
#include <QRadioButton>
#include <QLineEdit>

class UsereditDlg :public QWidget
{
	Q_OBJECT
public:
	UsereditDlg(QWidget* parent = nullptr);
	void init();

	void setUser(const QJsonObject& user);
signals:
	void userChanged(const QJsonObject&user);
private:
	QJsonObject m_juser;

	QLineEdit* m_user_id_edit{}; 
	QLineEdit* m_username_edit{};
	QLineEdit* m_mobile_edit{};
	QLineEdit* m_email_edit{};
	QRadioButton* m_man_btn{};
	QRadioButton* m_woman_btn{};

};

#endif // !USERADDDLG_H_
