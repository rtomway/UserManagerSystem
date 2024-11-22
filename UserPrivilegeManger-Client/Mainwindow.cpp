#include "Mainwindow.h"
#include <QBoxLayout>
#include <QFile>
#include <QSplitter>

#include "SMaskWidget.h"

#include "SConfigFile.h"
#include "SApp.h"
#include "SHttpClient.h"

#define HideBtn_width 20

enum NavType {
	NT_UserManger,
	NT_RoleManger,
	NT_PersonCenter
};

enum GradeType {
	GT_User,
	GT_GeneralManger,
	GT_HighestManger
};

Mainwindow::Mainwindow(QWidget*parent)
	:QWidget(parent)
{
	GetUserGrade();
		init();
	SMaskWidget::instance()->setMainWidget(this);
}
void Mainwindow::init()
{
	initMainwindow();
	initUserManagerPage();
	initPManagerPage();
	initPersonCenter();
	connect(m_personCenter, &PersonCenterPage::reLogin, this, &Mainwindow::reLogin);
}

void Mainwindow::initMainwindow()
{
	this->resize(1200,650);
	m_navBar = new SNavBar;
	m_treeNavBar = new QTreeWidget(this);
	m_stkWidget = new QStackedWidget(this);
	m_treeNavBar->setFixedWidth(270);
	//m_stkWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_hideBtn = new QPushButton(this);
	m_hideBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	updateBtnSize();
	m_hideBtn->setText("<");
	m_hideBtn->setObjectName("hideBtn");
	m_hideBtn->setStyleSheet(R"(
				QPushButton#hideBtn{
				background-color:rgb(243, 252, 249);
					color:white;
					font-size:15px;	
						}
				QPushButton#hideBtn:hover{
					background-color:rgb(228, 251, 243);
					}
			)");
	
	auto hlayout = new QHBoxLayout(this);
	hlayout->addWidget(m_treeNavBar);
	hlayout->addWidget(m_hideBtn);
	hlayout->addWidget(m_stkWidget);
	
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->setSpacing(0);
	m_treeNavBar->setFocusPolicy(Qt::NoFocus);

	//隐藏动画
	m_proAnimation = new QPropertyAnimation(m_treeNavBar, "geometry");
	m_proAnimation->setEasingCurve(QEasingCurve::Linear);
	m_MiddenAnimation= new QPropertyAnimation(m_hideBtn, "geometry");
	m_MiddenAnimation->setEasingCurve(QEasingCurve::Linear);
	m_RightAnimation = new QPropertyAnimation(m_stkWidget, "geometry");
	m_RightAnimation->setEasingCurve(QEasingCurve::Linear);
	hlayout->setGeometry(QRect(-m_treeNavBar->width(), 0,
		2 * m_treeNavBar->width() + m_stkWidget->width() + HideBtn_width, this->height()));
	connect(m_hideBtn, &QPushButton::clicked, [=]
		{
			hlayout->setGeometry(QRect( - m_treeNavBar->width(), 0,
									2 * m_treeNavBar->width() + m_stkWidget->width() + HideBtn_width,this->height()));
			hlayout->setEnabled(false);
			if (!m_treeIsHidden)
			{
				m_treeIsHidden = !m_treeIsHidden;
				m_proAnimation->setStartValue(QRect(0, 0, m_treeNavBar->width(), this->height()));
				m_proAnimation->setEndValue(QRect(-m_treeNavBar->width(), 0, m_treeNavBar->width(), this->height()));
				m_hideBtn->setText(">");
				m_MiddenAnimation->setStartValue(QRect(m_treeNavBar->width(), 0, m_hideBtn->width(), this->height()));
				m_MiddenAnimation->setEndValue(QRect(0, 0, m_hideBtn->width(), this->height()));
				m_RightAnimation->setStartValue(QRect(m_treeNavBar->width() + HideBtn_width, 0, m_stkWidget->width(), this->height()));
				m_RightAnimation->setEndValue(QRect(HideBtn_width, 0, this->width()-10, this->height()));
			}
			else
			{
				
				m_treeNavBar->show();
				m_treeIsHidden = !m_treeIsHidden;
			
				m_proAnimation->setStartValue(QRect(-m_treeNavBar->width(), 0, m_treeNavBar->width(), this->height()));
				m_proAnimation->setEndValue(QRect(0, 0, m_treeNavBar->width(), this->height()));

				m_hideBtn->setText("<");
				m_MiddenAnimation->setStartValue(QRect(0, 0, HideBtn_width, this->height()));
				m_MiddenAnimation->setEndValue(QRect(m_treeNavBar->width(), 0, HideBtn_width, this->height()));

				m_RightAnimation->setStartValue(QRect(HideBtn_width, 0, this->width(), this->height()));
				m_RightAnimation->setEndValue(QRect(m_treeNavBar->width() + HideBtn_width, 0, this->width() - m_treeNavBar->width() - 20, this->height()));
				
			}
			m_proAnimation->start();
			m_proAnimation->setDuration(200);
			m_MiddenAnimation->start();
			m_MiddenAnimation->setDuration(200);
			m_RightAnimation->start();
			m_RightAnimation->setDuration(200);
		});
	connect(m_proAnimation, &QPropertyAnimation::finished, [=]
		{
			if (m_treeIsHidden)
			{
				m_treeNavBar->hide();
			}	
				hlayout->setEnabled(true);
		});
	

	QTreeWidgetItem* item = nullptr;
	//头
	item = new QTreeWidgetItem({ "用户权限管理系统" });
	item->setIcon(0,QIcon(":/ResourceClient/privilege_header.png"));
	//item->setBackground(0, QBrush(QColor(57, 61, 72)));
	m_treeNavBar->setHeaderItem(item);
	//item
	auto item1 = new QTreeWidgetItem(m_treeNavBar, { "用户管理" }, NT_UserManger);
	item1->setIcon(0,QIcon(":/ResourceClient/user.png"));
	item1->setSelected(true);
	
	//
	/*if (m_Grade == 2)
	{
		qDebug() << "ppppppppppppppppppp";
		auto item2 = new QTreeWidgetItem(m_treeNavBar, { "角色管理" }, NT_RoleManger);
		item2->setIcon(0, QIcon(":/ResourceClient/role.png"));
	}*/

	auto item3 = new QTreeWidgetItem(m_treeNavBar, { "个人中心" }, NT_PersonCenter);
	item3->setIcon(0, QIcon(":/ResourceClient/privilege.png"));
	
	connect(m_treeNavBar, &QTreeWidget::itemClicked, [=](QTreeWidgetItem* item, int column)
		{
			switch (item->type())
			{
			case NT_UserManger:
				m_stkWidget->setCurrentIndex(NT_UserManger);
				//点击时刷新
				m_userManagerpage->onSearch();
				break;
			case NT_RoleManger:
				m_stkWidget->setCurrentIndex(NT_RoleManger);
				m_pManagerpage->onSearch();
				break;
			case NT_PersonCenter:
				m_stkWidget->setCurrentIndex(NT_PersonCenter);
				m_personCenter->GetpersonMessage();
				break;
			default:
				break;
			}
		});
	QFile file(":/ResourceClient/style.css");
	if (file.open(QIODevice::ReadOnly))
	{
		setStyleSheet(file.readAll());
	}
}

void Mainwindow::initUserManagerPage()
{
	m_userManagerpage = new UserManagerPage;
	m_stkWidget->addWidget(m_userManagerpage);
	
}

void Mainwindow::initPManagerPage()
{
	m_pManagerpage = new PManagerPage;
	m_stkWidget->addWidget(m_pManagerpage);
}

void Mainwindow::initPersonCenter()
{
	m_personCenter = new PersonCenterPage;
	m_stkWidget->addWidget(m_personCenter);
}

void Mainwindow::updateBtnSize()
{
	
	int buttonHeight = this->height();
	int buttonWidth = HideBtn_width;

	m_hideBtn->setGeometry(m_treeNavBar->geometry().x()+m_treeNavBar->width(),0, buttonWidth, buttonHeight);
	
}

void Mainwindow::updateReloginlater()
{
	m_personCenter->GetpersonMessage();
	GetUserGrade();
}

void Mainwindow::GetUserGrade()
{
	auto user_id = sApp->globalConfig()->value("user/user_id").toString();
	SHttpClient(URL("/api/user/list?user_id="+user_id)).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.success([=](const QByteArray& data)
			{
				QJsonParseError error;
				auto jdom = QJsonDocument::fromJson(data, &error);
				if (error.error != QJsonParseError::NoError)
				{
					qWarning() << "失败";
					return 0;
				}
				auto jarray = jdom.object().value("data").toObject().value("lists").toArray();
				QJsonObject juser = jarray.at(0).toObject();
				m_Grade=juser.value("grade").toInt();
				//获取用户权限等级
				m_userManagerpage->GetUserGrade(m_Grade);
				if (m_Grade == GT_HighestManger)
				{
					auto item2 = new QTreeWidgetItem(m_treeNavBar, { "角色管理" }, NT_RoleManger);
					item2->setIcon(0, QIcon(":/ResourceClient/role.png"));
					m_personCenter->showUserGrade("最高管理员");
				}
				else
				{
					if (m_Grade == GT_GeneralManger)
					{
						m_personCenter->showUserGrade("一般管理员");
					}
					else
					{
						m_personCenter->showUserGrade("普通用户");
					}
					m_treeNavBar->takeTopLevelItem(GT_HighestManger);
				}
			}).get();
	
}

void Mainwindow::resizeEvent(QResizeEvent* ev)
{
	updateBtnSize();
}

