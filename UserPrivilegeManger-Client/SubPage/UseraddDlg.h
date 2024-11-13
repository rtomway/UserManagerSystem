#ifndef USERADDDLG_H_
#define USERADDDLG_H_

#include <QWidget>
#include <QJsonObject>
#include <QRadioButton>
#include <QLineEdit>

class UseraddDlg :public QWidget
{
	Q_OBJECT
public:
	UseraddDlg(QWidget* parent = nullptr);
	void init();

	void setUser(const QJsonObject& user);
signals:
	void newUser(const QJsonObject&user);
private:
	QJsonObject m_juser;

	QLineEdit* m_user_id_edit{};
	QLineEdit* m_username_edit{};
	QLineEdit* m_mobile_edit{};
	QLineEdit* m_email_edit{};
	QRadioButton* m_def_password_RBtn{};
	QRadioButton* m_custom_password_RBtn{};
	QLineEdit* m_password_edit{};
};

#endif // !USERADDDLG_H_
