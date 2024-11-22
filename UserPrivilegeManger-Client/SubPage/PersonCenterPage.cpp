#include "PersonCenterPage.h"
#include "PersonCenterPage.h"
#include <QLayout>
#include "SHttpClient.h"
#include "SApp.h"


PersonCenterPage::PersonCenterPage(QWidget* parent)
	:PersonMessage(parent)
{
	init();
}

void PersonCenterPage::init()
{
	auto blayout = new QVBoxLayout(leftWidget());
	m_reLoginBtn = new QPushButton("重新登录");
	blayout->addWidget(m_reLoginBtn);
	blayout->addStretch();
	auto lab = new QLabel("<font size=2 face='宋体' color=gray>用户等级:</font>");
	blayout->addWidget(lab);
	m_gradeLabel = new QLabel;
	blayout->addWidget(m_gradeLabel);
	blayout->addStretch();

	connect(m_reLoginBtn, &QPushButton::clicked, this, &PersonCenterPage::reLogin);
	GetpersonMessage();
}

void PersonCenterPage::GetpersonMessage()
{
	//获取个人信息
	SHttpClient(URL("/api/user/list?user_id=" + sApp->globalConfig()->value("user/user_id").toString())).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.success([=](const QByteArray& data)
			{
				QJsonParseError error;
				auto jdom = QJsonDocument::fromJson(data, &error);
				if (error.error != QJsonParseError::NoError)
				{
					qWarning() << "失败";
					return;
				}
				auto jarray = jdom.object().value("data").toObject().value("lists").toArray();
				QJsonObject juser = jarray.at(0).toObject();
				juser.insert("user_id", juser.value("user_id").toString());
				juser.insert("username", juser.value("username").toString());
				juser.insert("gender", juser.value("gender").toInt() == 1 ? "男" : (juser.value("gender").toInt() == 2 ? "女" : "未知"));
				juser.insert("mobile", juser.value("mobile").toString());
				juser.insert("email", juser.value("email").toString());
				juser.insert("isEnable", juser.value("isEnable").toInt() == 1 ? true : false);
				setUser(juser);
			}).get();
}

void PersonCenterPage::showUserGrade(const QString& userGrade)
{
	m_gradeLabel->setText(userGrade);
}
