#include "UsereditDlg.h"
#include <QBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include "SHttpClient.h"
#include "SApp.h"
#include "SEventfilterObject2.h"
#include <QMessageBox>
#include <QButtonGroup>


UsereditDlg::UsereditDlg(QWidget* parent)
{
	init();
}

void UsereditDlg::init()
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
	m_man_btn = new QRadioButton("男");
	m_woman_btn = new QRadioButton("女");

	m_user_id_edit->setEnabled(false);

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
	auto radiolayout = new QHBoxLayout;
	radiolayout->addWidget(m_man_btn);
	radiolayout->addWidget(m_woman_btn);
	flayout->addRow("性别", radiolayout);
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
				m_juser.insert("user_id", m_user_id_edit->text());
				m_juser.insert("username", m_username_edit->text());
				m_juser.insert("mobile", m_mobile_edit->text());
				m_juser.insert("email", m_email_edit->text());
			
				if(m_man_btn->isChecked())
					m_juser.insert("gender",1);
				else if(m_woman_btn->isChecked())
					m_juser.insert("gender", 2);
				else
					m_juser.insert("gender", 0);


				SHttpClient(URL("/api/user/alter?user_id="+m_juser.value("user_id").toString())).debug(true)
					.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
					.json(m_juser)
					.success([=](const QByteArray& data)
						{
							QJsonParseError error;
							auto jdom = QJsonDocument::fromJson(data, &error);
							if ((error.error != QJsonParseError::NoError)&&jdom["code"].toInt()>1000)
							{
								return;
							}
							else
							{
								emit userChanged(m_juser);
								this->close();
							}
						}).post();

			});
		connect(cancelBtn, &QPushButton::clicked, [=]
			{
				this->close();
				
			});

	}
	
}

void UsereditDlg::setUser(const QJsonObject& user)
{
	m_juser = user;
	m_user_id_edit->setText(m_juser.value("user_id").toString());
	m_username_edit->setText(m_juser.value("username").toString());
	m_mobile_edit->setText(m_juser.value("mobile").toString());
	m_email_edit->setText(m_juser.value("email").toString());
	if (user.value("gender").toString() == "男")
		m_man_btn->setChecked(true);
		else if(user.value("gender").toString() == "女")
		m_woman_btn->setChecked(true);
		else
	{
		m_man_btn->setChecked(false);
		m_woman_btn->setChecked(false);
	}
		
}



