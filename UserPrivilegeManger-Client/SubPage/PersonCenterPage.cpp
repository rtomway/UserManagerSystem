#include "PersonCenterPage.h"
#include <QLayout>

PersonCenterPage::PersonCenterPage(QWidget* parent)
	:PersonMessage(parent)
{
	init();
}

void PersonCenterPage::init()
{
	auto blayout = new QVBoxLayout(leftLayout());
	auto reLoginBtn = new QPushButton("重新登录");
	blayout->addWidget(reLoginBtn);
	blayout->addStretch();
	
}
