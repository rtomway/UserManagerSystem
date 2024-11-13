#include "UserPrivilegeDlg.h"
#include "SHttpClient.h"
#include "SApp.h"

#include "SCheckDelegate.h"
#include "SHeaderView.h"
#include "SFieldTranslate.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QFormLayout>
#include <QTableView>

UserPrivilegeDlg::UserPrivilegeDlg(QWidget* parent)
	:QWidget(parent)
{
	init();
}

void UserPrivilegeDlg::init()
{

	m_user_id_edit = new QLineEdit;
	m_user_id_edit ->setReadOnly(true);

	m_privilege_search_edit = new QLineEdit;

	auto mlayout = new QVBoxLayout(this);

	auto flayout = new QFormLayout();
	flayout->setRowWrapPolicy(QFormLayout::WrapAllRows);

	flayout->setWidget(0, QFormLayout::ItemRole::SpanningRole, new QLabel("<div style='background-color:rgb(253,246,219);font-family:'黑体';'>用户加入到权限后，讲拥有该权限的所有权限</div>"));
	
	flayout->addRow("*用户ID", m_user_id_edit);
	flayout->addRow("*查找权限", m_privilege_search_edit);

	m_tableView = new QTableView;
	m_tableView->setAlternatingRowColors(true);
	m_tableView->setHorizontalHeader(new SHeaderView(Qt::Orientation::Horizontal));
	//m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
	m_tableView->horizontalHeader()->setHighlightSections(true);
	m_tableView->verticalHeader()->hide();
	m_tableView->setShowGrid(false);
	m_tableView->setFocusPolicy(Qt::NoFocus);
	m_tableView->setFrameShape(QFrame::NoFrame);
	m_tableView->setSelectionMode(QTableView::SingleSelection);
	m_tableView->setEditTriggers(QTableView::NoEditTriggers);
	m_tableView->setSelectionBehavior(QTableView::SelectRows);
	m_tableView->setMouseTracking(true);

	auto checkDelegate = new SCheckDelegate(this);
	m_tableView->setItemDelegateForColumn(0, checkDelegate);
	//....	


	 m_selectModel = new QStandardItemModel(this);
	 getAllPriviegeList();
	m_tableView->setModel(m_selectModel);
	
	flayout->addRow("<font color=red>请选择权限</font>",m_tableView);
	mlayout->addLayout(flayout);
	
	//底部按钮栏
	{
		auto okBtn = new QPushButton("确定");
		auto cancelBtn = new QPushButton("取消");

		auto bhlayout = new QHBoxLayout;
		bhlayout->addStretch();
		bhlayout->addWidget(okBtn);
		bhlayout->addWidget(cancelBtn);

		mlayout->addStretch();
		mlayout->addLayout(bhlayout);

		connect(okBtn, &QPushButton::clicked, [=]()
			{
				SHttpClient(URL("/api/privilege/create")).debug(true)
					.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
					.json(m_juser)
					.success([=](const QByteArray& data)
						{
							QJsonParseError error;
							auto jdom = QJsonDocument::fromJson(data, &error);
							if (error.error != QJsonParseError::NoError && jdom["code"].toInt() > 1000) {
								return;
							}
							else {
							}
						}).post();


			});
		connect(cancelBtn, &QPushButton::clicked, [=]()
			{
				this->close();
			});
	}
}

void UserPrivilegeDlg::setUser(const QJsonObject& juser)
{
	m_juser = juser;
	m_user_id_edit->setText(m_juser.value("user_id").toString());

}

void UserPrivilegeDlg::getAllPriviegeList()
{
	QVariantMap params;
	auto filter = m_privilege_search_edit->text();
	if (!filter.isEmpty())
		params.insert("query", filter);

	//params.insert("pageSize", 10000);

	SHttpClient(URL("/api/privilege/list")).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.params(params)
		.success([=](const QByteArray& data)
			{
				QJsonParseError error;
				auto jdom = QJsonDocument::fromJson(data, &error);
				if (error.error != QJsonParseError::NoError) {
					return;
				}
				parseJson(jdom.object());

			}).get();

}

void UserPrivilegeDlg::parseJson(const QJsonObject& obj)
{
	m_selectModel->clear();

	for (size_t i = 0; i < m_fieldName.size(); i++) {
		m_selectModel->setHorizontalHeaderItem(i, new QStandardItem(SFieldTrandlate::instance()->trans("privilege", m_fieldName[i])));
	}

	auto jarray =  obj.value("data").toObject().value("lists").toArray();
	for (int i = 0; i < jarray.size(); i++)
	{
		auto juser = jarray.at(i).toObject();

		m_selectModel->appendRow(ItemsFromJsonObject(juser));
	}

	m_tableView->setColumnWidth(0,26);
	dynamic_cast<SHeaderView*>(m_tableView->horizontalHeader())->setState(Qt::Unchecked);
}

QList<QStandardItem*> UserPrivilegeDlg::ItemsFromJsonObject(const QJsonObject& jobj)
{
	QList<QStandardItem*> items;
	for (const auto& field : m_fieldName)
	{
		auto item = new QStandardItem;
		item->setTextAlignment(Qt::AlignCenter);
		item->setText(jobj.value(field).toVariant().toString());
		items.append(item);
	}
	return items;
}


