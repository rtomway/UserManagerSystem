#ifndef _LOGINREGISTERDLG_H_
#define _LOGINREGISTERDLG_H_

#include<QDialog>
#include <QNetworkAccessManager>

class QLabel;
class QStackedWidget;
class QLineEdit;
class QCheckBox;
class QRadioButton;
class QByteArray;
class QComboBox;

class LoginRegisterDlg : public QDialog
{
	Q_OBJECT
public:
	LoginRegisterDlg(QWidget* parent = nullptr);
	~LoginRegisterDlg();
private:
	void initUi();
	QLineEdit* m_accountEdit{};
	QLineEdit* m_passwordEdit{};
	QCheckBox* m_rememberPwdChx{};
	QCheckBox* m_autoLoginChx{};
	QWidget * CreateLoginWidget();

	QLineEdit* m_regit_accountEdit{};
	QLineEdit* m_regit_passwordEdit{};
	QLineEdit* m_regit_rePasswordEdit{};
	QLineEdit*   m_regit_nicknameEdit{};
	QComboBox*   m_regit_postCbx{};

	QWidget* CreateRegisterWidget();
signals:
	 void loginSuccess();
public://slots
	void onLogin();
	void onRegister();
protected:
	void paintEvent(QPaintEvent* ev)override;
private:
	QRect m_rightRect;
	QRect m_leftRect;
	QRect m_middleRect;

	QStackedWidget* m_stkWidget{};
	QWidget* m_loginWidget{};
	QWidget* m_registerWidget{};

	bool m_isLogining{};

	QLabel* m_tipBox{};
	QTimer* m_timer{};
	void setTip(const QString& msg);

	QNetworkAccessManager* m_manager{};
};

#endif // !_LOGINREGISTERDLG_H_
