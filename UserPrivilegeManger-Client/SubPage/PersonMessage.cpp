#include "PersonMessage.h"
#include <QBoxLayout>
#include <QLabel>
#include <QFormLayout>

#include <QFile>
#include "SApp.h"
#include "SHttpClient.h"
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include <QHttpMultipart>
#include "SMaskWidget.h"
#include <QDebug>

PersonMessage::PersonMessage(QWidget* parent)
	:QWidget(parent)
	,m_leftLayout(new QWidget)
{
	init();
}

void PersonMessage::init()
{
	//本身背景样式
	this->setAttribute(Qt::WA_StyledBackground);
	setStyleSheet(R"(
				QLabel
				{
					font-size:15px;	
				}
				QPushButton
				{
					width:80px;
					height:30px;
				}
			)");
	auto thlayout = new QHBoxLayout(this);
	//auto mmlayout= new QVBoxLayout;
	thlayout->addWidget(leftLayout());
	//thlayout->addStretch(0);
	auto rightLatout = new QWidget;
	auto mlayout = new QVBoxLayout(rightLatout);
	thlayout->addWidget(rightLatout);
	//thlayout->addStretch();

	//m_backBtn = new QPushButton("返回");
	//m_backBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	////m_backBtn->setFixedSize(60, 25);
	//mmlayout->addWidget(m_backBtn);
	//mmlayout->addStretch();

	auto baseInfoBtn = new QPushButton("编辑基本信息");
	auto resetPasswordBtn = new QPushButton("重置密码");
	auto delBtn = new QPushButton("删除用户");
	auto avatarAlterBtn = new QPushButton("头像修改");

	m_avatar_lab = new QLabel;
	m_user_id_lab = new QLabel;
	m_username_lab = new QLabel;
	m_mobile_lab = new QLabel;
	m_email_lab = new QLabel;
	m_gender_lab = new QLabel;
	m_isEnable_btn = new SSwitchButton;
	m_isEnable_btn->setFixedWidth(40);

	mlayout->addSpacerItem(new QSpacerItem(0, 50));
		
		mlayout->addSpacerItem(new QSpacerItem(1, 30));
		m_avatar_lab->setPixmap(QPixmap(":/ResourceClient/default_avatar.jpg"));
		m_avatar_lab->setFixedSize(150, 150);
		m_avatar_lab->setScaledContents(true);
		m_avatar_lab->setMask(QRegion(m_avatar_lab->rect(), QRegion::RegionType::Ellipse));

		auto alayout = new QHBoxLayout;
		alayout->addWidget(m_avatar_lab);
		alayout->addWidget(avatarAlterBtn);
		alayout->addStretch();

		mlayout->addLayout(alayout);

		mlayout->addSpacerItem(new QSpacerItem(60, 30));

		auto lflayout = new QFormLayout;
		lflayout->addRow("用户ID", m_user_id_lab);
		lflayout->addRow("用户名", m_username_lab);
		lflayout->addRow("性别", m_gender_lab);


		auto rflayout = new QFormLayout;
		rflayout->addRow("电话", m_mobile_lab);
		rflayout->addRow("邮箱", m_email_lab);
		rflayout->addRow("账号状态", m_isEnable_btn);


		auto hhlayout = new QHBoxLayout;
		hhlayout->addLayout(lflayout);
		hhlayout->addSpacerItem(new QSpacerItem(60, 20));
		hhlayout->addLayout(rflayout);
		hhlayout->addStretch();

		mlayout->addLayout(hhlayout);

		auto hlayout = new QHBoxLayout;
		hlayout->addWidget(baseInfoBtn);
		hlayout->addSpacerItem(new QSpacerItem(30, 20));
		hlayout->addWidget(resetPasswordBtn);
		hlayout->addSpacerItem(new QSpacerItem(30, 20));
		hlayout->addWidget(delBtn);
		hlayout->addStretch();
		

		mlayout->addSpacerItem(new QSpacerItem(60,20));
		mlayout->addLayout(hlayout);
		mlayout->addStretch();
	
	
	/*connect(m_backBtn, &QPushButton::clicked, [=]
		{
			emit setshow();
		});*/
	connect(avatarAlterBtn, &QPushButton::clicked, this, &PersonMessage::onAvatarUpload);
	connect(m_isEnable_btn, &SSwitchButton::stateChanged, [=](bool state)
		{
			SHttpClient(URL("/api/user/alter?user_id="+m_juser.value("user_id").toString())).debug(true)
				.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
				.json({ {"isEnable",state}})
				.fail([=](const QString& msg, int code)
					{
						
					})
				.success([=](const QByteArray& data)
					{
						QJsonParseError error;
						auto jdom = QJsonDocument::fromJson(data, &error);
						if (error.error != QJsonParseError::NoError)
						{
							
						}
						else
						{
							if (jdom["code"].toInt() == 0)
							{
								qDebug() << "1111111111111111"<<m_juser;
								m_juser.insert("isEnable", state);
								qDebug() << "00000000000000000" << m_juser;
								emit userChanged(m_juser);
							}
							
						}

					})
				.post();
		});
	connect(delBtn, &QPushButton::clicked, [=]
		{
			QJsonArray jarray;
			jarray.append(m_juser.value("user_id").toString());
			SHttpClient(URL("/api/user/delete")).debug(true)
				.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
				.json({ {"lists",jarray} })
				.fail([](const QString& msg, int code)
					{
						qDebug() << msg << code;
					})
				.success([=](const QByteArray& data)
					{
						QJsonParseError err;
						auto jdom = QJsonDocument::fromJson(data, &err);
						if (jdom["code"].toInt() == 0)
						{
							emit setshow();
							emit userDeleted(m_juser);
						}
					})
				.post();
		});
	connect(baseInfoBtn, &QPushButton::clicked, [=]()
		{
			if (!m_userEditDlg)
			{
				m_userEditDlg = new UsereditDlg;
				connect(m_userEditDlg, &UsereditDlg::userChanged, [=](const QJsonObject& user)
					{
						m_juser = user;
						m_juser.insert("gender", user.value("gender").toInt() == 1 ? "男" : (user.value("gender").toInt() == 2?"女" : "未知"));
						setUser(m_juser);
						emit userChanged(m_juser);
					});
			}
			m_userEditDlg->setUser(m_juser);
			SMaskWidget::instance()->popUp(m_userEditDlg);
		});
}

void PersonMessage::setUser(const QJsonObject& user)
{
	
	m_juser = user;
	m_user_id_lab->setText(m_juser.value("user_id").toString());
	m_username_lab->setText(m_juser.value("username").toString());
	m_gender_lab->setText(m_juser.value("gender").toString());
	m_isEnable_btn->setToggle(m_juser.value("isEnable").toBool());
	m_mobile_lab->setText(m_juser.value("mobile").toString());
	m_email_lab->setText(m_juser.value("email").toString());
	onAvatarDownload();
}

QWidget* PersonMessage::leftLayout() const
{

	return m_leftLayout;
}


void PersonMessage::onAvatarDownload()
{
	SHttpClient(URL("/api/user/avatar")).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.param("user_id",m_juser.value("user_id"))
		.fail([=](const QString& msg, int code)
			{
				m_avatar_lab->setPixmap(QPixmap(":/ResourceClient/default_avatar.jpg"));
				m_avatar_lab->setMask(QRegion(m_avatar_lab->rect(), QRegion::RegionType::Ellipse));
			})
		.success([=](const QByteArray& data)
			{
				if (data.startsWith('{'))
				{
					m_avatar_lab->setPixmap(QPixmap(":/ResourceClient/default_avatar.jpg"));
					m_avatar_lab->setMask(QRegion(m_avatar_lab->rect(), QRegion::RegionType::Ellipse));
				}
				else
				{
					auto img = QImage::fromData(data);
					m_avatar_lab->setPixmap(QPixmap::fromImage(img));
					m_avatar_lab->setMask(QRegion(m_avatar_lab->rect(), QRegion::RegionType::Ellipse));
				}
			})
		.get();
}

void PersonMessage::onAvatarUpload()
{
	auto path=sApp->globalConfig()->value("other/select_avatar_path",
		QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
	auto filename = QFileDialog::getOpenFileName(this, "选择头像", path, "images (*.jpg;*.jpeg;*.png)");
	if (filename.isEmpty())
		return;
	sApp->globalConfig()->setValue("other/select_avatar_path", QFileInfo(filename).canonicalPath());

	//头像大小
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(this, "失败", "头像打开失败");
		return;
	}
	/*if (file.size() > 3 * 1024 * 1024)
	{
		QMessageBox::warning(this, "失败", "图像尺寸过大");
		return;
	}*/


	auto url = URL("/api/user/avatar?user_id=" + m_juser.value("user_id").toString());
	QNetworkAccessManager* mangaer = new QNetworkAccessManager(this);
	QNetworkRequest request(url);
	request.setRawHeader("Authorization", "Bearer" + sApp->userData("user/token").toString().toUtf8());

	QHttpMultiPart* mpart = new QHttpMultiPart(this);
	QHttpPart part;
	part.setBody(file.readAll());
	part.setHeader(QNetworkRequest::ContentTypeHeader, QString("image/%1").arg(QFileInfo(filename).suffix()));
	part.setHeader(QNetworkRequest::ContentDispositionHeader,
		QString(R"(attachment;name="file";filename="%1")").arg(QFileInfo(filename).fileName()));
	mpart->append(part);
	auto reply=mangaer->post(request,mpart);
	connect(reply, &QNetworkReply::finished, [=]
		{
			if (reply->error() != QNetworkReply::NoError)
			{
				qDebug() << "reply error" << reply->errorString();
				m_avatar_lab->setPixmap(QPixmap(":/ResourceClient/default_avatar.jpg"));
				m_avatar_lab->setMask(QRegion(m_avatar_lab->rect(), QRegion::RegionType::Ellipse));
			}
			else
			{
				qDebug() <<QString(reply->readAll());
				m_avatar_lab->setPixmap(filename);
				m_avatar_lab->setMask(QRegion(m_avatar_lab->rect(), QRegion::RegionType::Ellipse));
			}
		});
}

