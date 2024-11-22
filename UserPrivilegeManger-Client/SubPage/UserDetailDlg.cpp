#include "UserDetailDlg.h"
#include <QBoxLayout>

UserDetailDlg::UserDetailDlg(QWidget* parent)
	:PersonMessage(parent)
{
	init();
}

void UserDetailDlg::init()
{
	auto mlayout = new QVBoxLayout(leftWidget());
	m_backBtn = new QPushButton("返回");
	mlayout->addWidget(m_backBtn);
	mlayout->addStretch();

	connect(m_backBtn, &QPushButton::clicked, [=]
		{
			this->hide();
		});
}
