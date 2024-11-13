#include "UseraddDlg.h"
#include <QBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include "SHttpClient.h"
#include "SApp.h"
#include "SEventfilterObject2.h"
#include <QMessageBox>


UseraddDlg::UseraddDlg(QWidget* parent)
{
	init();
}

void UseraddDlg::init()
{
	setStyleSheet(R"(
	    QWidget{
			background-color:rgb(240, 254, 250) ;
			}
		QPushButton {
          background-color:white;
          border: 1px solid #211c1c;
          border-radius: 5px;       
          padding: 10px 20px;      
          box-shadow: 0 4px 0 #DDDDDD;
        }
        QPushButton:hover {
           background-color: rgb(220, 254, 250);
        }
        QPushButton:pressed {
           background-color: rgb(240, 254, 250);
           border: 1px solid  #DDDDDD;
           box-shadow: inset 0 2px 5px rgba(0, 0, 0, 0.5);
        }
	)");
	setAttribute(Qt::WA_StyledBackground);
	setAttribute(Qt::WA_StyleSheet);

	m_user_id_edit = new QLineEdit;
	m_username_edit = new QLineEdit;
	m_mobile_edit = new QLineEdit;
	m_email_edit = new QLineEdit;
	m_def_password_RBtn = new QRadioButton("默认密码");
	m_custom_password_RBtn = new QRadioButton("自定义密码");
	m_password_edit = new QLineEdit;

	auto mlayout = new QVBoxLayout(this);
	mlayout->setSizeConstraint(QLayout::SetFixedSize);

	//installEventFilter(new SEventFilterObject2(this));

	auto flayout = new QFormLayout;
	flayout->setRowWrapPolicy(QFormLayout::RowWrapPolicy::WrapAllRows);

	flayout->addRow("<font color=red>*</font>用户ID", m_user_id_edit);
	auto lab1 = new QLabel("<font size=2 face='宋体' color=gray>最大长度64位,允许英文字母,数字</font>");
	flayout->addWidget(lab1);
	flayout->addRow("<font color=red>*</font>用户名", m_username_edit);
	auto lab2 = new QLabel("<font size=2 face='宋体' color=gray>最大长度64位,允许英文字母,数字</font>");
	flayout->addWidget(lab2);
	flayout->addRow("<font color=red>*</font>电话号码", m_mobile_edit);
	auto lab3 = new QLabel("<font size=2 face='宋体' color=gray>最大长度64位,允许英文字母,数字</font>");
	flayout->addWidget(lab3);
	flayout->addRow("电子邮箱", m_email_edit);
	auto lab4 = new QLabel("<font size=2 face='宋体' color=gray>最大长度64位,允许英文字母,数字</font>");
	flayout->addWidget(lab4);
	
	mlayout->addLayout(flayout);

	{
		auto hlayout = new QHBoxLayout;
		auto cancelBtn = new QPushButton("取消");
		auto okBtn = new QPushButton("确定");

		//hlayout->addStretch();
		hlayout->addWidget(okBtn);
		hlayout->addWidget(cancelBtn);

		mlayout->addSpacerItem(new QSpacerItem(this->width(), 50));
		mlayout->addLayout(hlayout);
		mlayout->addStretch();
		connect(okBtn, &QPushButton::clicked, [=]
			{
				QString password = m_password_edit->text();
				if (m_def_password_RBtn->isChecked())
				{
					password = "123456";
				}
				m_juser.insert("user_id", m_user_id_edit->text());
				m_juser.insert("username", m_username_edit->text());
				m_juser.insert("mobile", m_mobile_edit->text());
				m_juser.insert("email", m_email_edit->text());
				m_juser.insert("password",password);

				qDebug() << m_juser;
				SHttpClient(URL("/api/user/create")).debug(true)
					.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
					.json(m_juser)
					.success([=](const QByteArray& data)
						{
							QJsonParseError error;
							auto jdom = QJsonDocument::fromJson(data, &error);
							if ((error.error != QJsonParseError::NoError)&&jdom["code"].toInt()>1000)
							{
								qDebug() << "--------------" << (error.error != QJsonParseError::NoError) << jdom["code"].toInt();
								QMessageBox::critical(this,QString("无法创建"), jdom["message"].toString());
								return;
							}
							else
							{
								emit newUser(m_juser);
							}
						 
						}).post();

			});
		connect(cancelBtn, &QPushButton::clicked, [=]
			{
				this->close();
			});

	}
	
	m_def_password_RBtn->setChecked(true);
	m_password_edit->hide();
	connect(m_custom_password_RBtn, &QRadioButton::toggled, [=](bool Checked)
		{
			if(Checked)
			m_password_edit->show();
			else
			m_password_edit->hide();
		});

	auto hlayout = new QHBoxLayout;
	hlayout->addWidget(m_def_password_RBtn);
	hlayout->addWidget(m_custom_password_RBtn);

	flayout->addRow("初始密码",hlayout);
	flayout->addWidget(m_password_edit);
}

void UseraddDlg::setUser(const QJsonObject& user)
{
	m_juser = user;
	/*m_backBtn->setText(m_juser.value("username").toString());
	m_user_id_lab->setText(m_juser.value("user_id").toString());
	m_username_lab->setText(m_juser.value("username").toString());
	m_isEnable_btn->setToggle(m_juser.value("isEnable").toBool());
	m_mobile_lab->setText(m_juser.value("mobile").toString());
	m_email_lab->setText(m_juser.value("email").toString());*/
}



