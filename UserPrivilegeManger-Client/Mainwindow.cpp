#include "Mainwindow.h"
#include <QBoxLayout>
#include <QFile>
#include <QSplitter>

#include "SMaskWidget.h"

#define HideBtn_width 20

enum NavType {
	NT_UserManger,
	NT_RoleManger,
	NT_PrivilegeManger
};

Mainwindow::Mainwindow(QWidget*parent)
	:QWidget(parent)
{
	init();
	SMaskWidget::instance()->setMainWidget(this);
}
void Mainwindow::init()
{
	initMainwindow();
	initUserManagerPage();
	initPManagerPage();
}

void Mainwindow::initMainwindow()
{
	this->resize(1000,650);

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
					font-color:white;
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
	item = new QTreeWidgetItem(m_treeNavBar, { "用户管理" }, NT_UserManger);
	item->setIcon(0,QIcon(":/ResourceClient/user.png"));
	item->setSelected(true);
	
	item = new QTreeWidgetItem(m_treeNavBar, { "角色管理" }, NT_RoleManger);
	item->setIcon(0, QIcon(":/ResourceClient/role.png"));
	item = new QTreeWidgetItem(m_treeNavBar, { "权限管理" }, NT_PrivilegeManger);
	item->setIcon(0, QIcon(":/ResourceClient/privilege.png"));
	connect(m_treeNavBar, &QTreeWidget::itemClicked, [=](QTreeWidgetItem* item, int column)
		{
			switch (item->type())
			{
			case NT_UserManger:
				m_stkWidget->setCurrentIndex(NT_UserManger);
				break;
			case NT_RoleManger:
				m_stkWidget->setCurrentIndex(NT_RoleManger);
				m_pManagerpage->onSearch();
				break;
			case NT_PrivilegeManger:
				m_stkWidget->setCurrentIndex(NT_PrivilegeManger);
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

void Mainwindow::updateBtnSize()
{
	
	int buttonHeight = this->height();
	int buttonWidth = HideBtn_width;

	m_hideBtn->setGeometry(m_treeNavBar->geometry().x()+m_treeNavBar->width(),0, buttonWidth, buttonHeight);
	
}

void Mainwindow::resizeEvent(QResizeEvent* ev)
{
	updateBtnSize();
}

