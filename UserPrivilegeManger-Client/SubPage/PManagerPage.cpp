#include "PManagerPage.h"
#include "PManagerPage.h"
#include "PManagerPage.h"
#include "ui_PManagerPage.h"

#include "SApp.h"
#include "SButtonDelegate.h"
#include "SCheckDelegate.h"
#include "SFieldTranslate.h"
#include "SHeaderView.h"
#include "SHttpClient.h"
#include "SSwitchDelegate.h"
#include "SUrlDelegate.h"

#include "SMaskWidget.h"
#include "PAddEditDlg.h"
#include "PersonMessage.h"
#include "UseraddDlg.h"
#include "UserManagerPage.h"

#include "xlsxdocument.h"
#include "xlsxworkbook.h"
#include "xlsxworksheet.h"

#include <QFile>
#include <QFileDialog>
#include <set>

PManagerPage::PManagerPage(QWidget* parent)
	:QWidget(parent)
	,ui(new Ui::PManagerPage)
	,m_model(new QStandardItemModel(this))
{
	ui->setupUi(this);
	init();
}

PManagerPage::~PManagerPage()
{
	delete ui;
}

void PManagerPage::init()
{
	SFieldTrandlate::instance()->addTrans("user_privilege/user_id", "用户ID");
	SFieldTrandlate::instance()->addTrans("user_privilege/username", "用户名");
	SFieldTrandlate::instance()->addTrans("user_privilege/privilege_edit", "权限_编辑");
	SFieldTrandlate::instance()->addTrans("user_privilege/privilege_add", "权限_添加");
	SFieldTrandlate::instance()->addTrans("user_privilege/privilege_delete", "权限_删除");

	initMenu();
	//表格设置
	ui->tableView->setModel(m_model);
	ui->tableView->setSelectionBehavior(QTableView::SelectRows);
	ui->tableView->setSelectionMode(QTableView::NoSelection);
	ui->tableView->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->tableView->setEditTriggers(QTableView::NoEditTriggers);
	ui->tableView->setMouseTracking(true);

	connect(ui->exportBtn, &QPushButton::clicked, this, &PManagerPage::onExport);
	//搜索
	connect(ui->searchEdit, &QLineEdit::returnPressed, this, &PManagerPage::onSearch);
	connect(ui->searchEdit, &QLineEdit::textChanged, [=](const QString& text) {
		if (text.isEmpty())
			onSearch();
		});
	connect(ui->searchBtn, &QPushButton::clicked, this, &PManagerPage::onSearch);
	//界面创建的时候，查询一下
	onSearch();

	//连接批量操作按钮信号
	//connect(ui->batchDeleteBtn, &QPushButton::clicked, this, &PManagerPage::onBatchDelete);

	auto switchDelegate = new SSwitchDelegate(ui->tableView);
	ui->tableView->setItemDelegateForColumn(column("privilege_edit"), switchDelegate);
	ui->tableView->setItemDelegateForColumn(column("privilege_add"), switchDelegate);
	ui->tableView->setItemDelegateForColumn(column("privilege_delete"), switchDelegate);
	connect(switchDelegate, &SSwitchDelegate::stateChanged, [=](bool state, const QModelIndex& index)
		{
			auto user_id = m_model->item(index.row(), column("user_id"))->text();

			SHttpClient(URL("/api/user_privilege/alter?user_id=" + user_id)).debug(true)
				.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
				.json({ {"privilege_edit",state} })
				.fail([](const QString& msg, int code)
					{
						qDebug() << msg << code;
					})
				.success([=](const QByteArray& data)
					{
						QJsonParseError err;
						auto jdom = QJsonDocument::fromJson(data, &err);
						if (jdom["code"].toInt() < 1000)
						{

						}
					})
				.post();
			SHttpClient(URL("/api/user_privilege/alter?user_id=" + user_id)).debug(true)
				.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
				.json({ {"privilege_add",state} })
				.fail([](const QString& msg, int code)
					{
						qDebug() << msg << code;
					})
				.success([=](const QByteArray& data)
					{
						QJsonParseError err;
						auto jdom = QJsonDocument::fromJson(data, &err);
						if (jdom["code"].toInt() < 1000)
						{

						}
					})
				.post();
			SHttpClient(URL("/api/user_privilege/alter?user_id=" + user_id)).debug(true)
				.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
				.json({ {"privilege_delete",state} })
				.fail([](const QString& msg, int code)
					{
						qDebug() << msg << code;
					})
				.success([=](const QByteArray& data)
					{
						QJsonParseError err;
						auto jdom = QJsonDocument::fromJson(data, &err);
						if (jdom["code"].toInt() < 1000)
						{

						}
					})
				.post();
		});

	//添加用户时
	/*connect(,&UseraddDlg::newUser, [=](const QJsonObject& juser)
		{
			m_model->insertRow(0, ItemsFromJsonObject(juser));
		});*/
}

void PManagerPage::onSearch()
{
	QVariantMap params;
	auto filter = ui->searchEdit->text();
	if (!filter.isEmpty())
		params.insert("query", filter);

	//params.insert("pageSize", 10000);

	SHttpClient(URL("/api/user_privilege/list")).debug(true)
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

void PManagerPage::onBatchDelete()
{
	QJsonObject jobj;
	QJsonArray jarr;
	//判断有哪些行是被选中的
	std::set<int, std::greater<int>> deleteRows;
	for (size_t i = 0; i < m_model->rowCount(); i++)
	{
		auto item =  m_model->item(i);
		if (item && item->data(Qt::UserRole).toBool())//&& item->checkState() == Qt::CheckState::Checked)
		{
			jarr.append(m_model->item(i,column("name"))->text());
			deleteRows.insert(i);
		}
	}
	jobj.insert("lists", jarr);

	SHttpClient(URL("/api/privilege/delete")).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.json(jobj)
		.fail([](const QString& msg, int code)
			{
			})
		.success([=](const QByteArray& data)
			{
				for (auto row : deleteRows) {
					m_model->removeRow(row);
				}
			})
		.post();
}

void PManagerPage::parseJson(const QJsonObject& obj)
{
	m_model->clear();

	for (size_t i = 0; i < m_fieldName.size(); i++) {
		m_model->setHorizontalHeaderItem(i, new QStandardItem(SFieldTrandlate::instance()->trans("user_privilege", m_fieldName[i])));
	}

	auto jarray =  obj.value("data").toObject().value("lists").toArray();
	for (int i = 0; i < jarray.size(); i++)
	{
		auto juser = jarray.at(i).toObject();

		m_model->appendRow(ItemsFromJsonObject(juser));
	}
	ontableViewsize();

}

void PManagerPage::ontableViewsize()
{
	int columnCount = m_model->columnCount();
	int totalWidth = ui->tableView->width()- 18;
	if (columnCount > 0) {
		int columnWidth = totalWidth / (columnCount);
		for (int i = 0; i < columnCount; i++) {
			ui->tableView->setColumnWidth(i, columnWidth);
			ui->tableView->setRowHeight(i, 40);
		}
	}
}

void PManagerPage::resizeEvent(QResizeEvent* ev)
{
	ontableViewsize();
}

int PManagerPage::column(const QString& field)
{
	auto pos =  m_fieldName.indexOf(field);
	if (pos == -1) {
		qWarning() << "field " << field << "not exists!";
	}
	return pos;
}

QList<QStandardItem*> PManagerPage::ItemsFromJsonObject(const QJsonObject& jobj)
{
	auto privilege_edit = jobj.value("privilege_edit").toInt();
	auto privilege_add = jobj.value("privilege_add").toInt();
	auto privilege_delete = jobj.value("privilege_delete").toInt();
	
	QList<QStandardItem*> items;
	for (const auto& field : m_fieldName)
	{
		auto item = new QStandardItem;
		item->setTextAlignment(Qt::AlignCenter); 

		if (field == "privilege_edit")
		{
			item->setText(privilege_edit ? "正常" : "禁用");
			item->setData(privilege_edit, Qt::UserRole);
		}
		else if (field == "privilege_add")
		{
			item->setText(privilege_add ? "正常" : "禁用");
			item->setData(privilege_add, Qt::UserRole);
		}
		else if (field == "privilege_delete")
		{
			item->setText(privilege_delete ? "正常" : "禁用");
			item->setData(privilege_delete, Qt::UserRole);
		}
		else
		{
			item->setText(jobj.value(field).toVariant().toString());
		}
		items.append(item);
	}
	return items;
}

void PManagerPage::initMenu()
{
	m_importMenu = new QMenu(this);
	ui->addBtn->setMenu(m_importMenu);

	m_importMenu->addAction("单条导入",this,&PManagerPage::singleImport);
	m_importMenu->addAction("批量导入",this,&PManagerPage::batchImport);
}

void PManagerPage::singleImport()
{
	if (!m_pAddEditDlg) {
		m_pAddEditDlg = new PAddEditDlg;
		connect(m_pAddEditDlg, &PAddEditDlg::newPrivilege, [=](const QJsonObject& juser)
			{
				m_model->insertRow(0, ItemsFromJsonObject(juser));
				m_pAddEditDlg->close();
			});
	}
	SMaskWidget::instance()->popUp(m_pAddEditDlg);

}

void PManagerPage::batchImport()
{
	auto filename = QFileDialog::getOpenFileName(this, "选择权限表格", "./", "xlsx (*.xlsx;*.csv);;all (*.*)");
	if (filename.isEmpty()) {
		return;
	}

	//判断是什么类型的文件csv xlsx
	auto extraName = QFileInfo(filename).suffix();
	if (extraName.compare("csv", Qt::CaseInsensitive) == 0)
	{
		readCSVFile(filename);
	}
	else if (extraName.compare("xlsx", Qt::CaseInsensitive) == 0)
	{
		readXLSXFile(filename);
	}
}

void PManagerPage::onExport()
{
	auto filename = QFileDialog::getSaveFileName(this, "导出", "untitled.csv", "csv (*.csv);;xlsx (*.xlsx);;all (*.*)");
	if (filename.isEmpty())
		return;
	//判断是什么类型的文件csv xlsx
	auto extraName = QFileInfo(filename).suffix();
	if (extraName.compare("csv", Qt::CaseInsensitive) == 0) {
		writeCSVFile(filename);
	}
	else if (extraName.compare("xlsx", Qt::CaseInsensitive) == 0) {
		writeXLSXFile(filename);
	}
}

void PManagerPage::readCSVFile(const QString& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning() << filename << "open failed:" << file.errorString();
		return;
	}

	QTextStream stream(&file);
	auto guessBom = file.read(3);
	if (guessBom.startsWith("\xEF\xBB\xBf")) {
		stream.setEncoding(QStringConverter::Utf8);
	}
	else {
		stream.setEncoding(QStringConverter::System);
	}

	auto headers = stream.readLine().split(',');

	QJsonArray jarr;
	while (!stream.atEnd())
	{
		auto lineData =  stream.readLine().split(',');
		if (lineData.size() != 2) {
			qWarning() << "表头字段数量不正确，导入失败";
			break;
		}
		auto gender = lineData.at(2) == "未知" ? 0 : lineData.at(2) == "男" ? 1 : 2;

		QJsonObject jobj;
		jobj.insert("user_id", lineData.at(0));
		jobj.insert("username", lineData.at(1));
		jobj.insert("gender", gender);
		jobj.insert("mobile", lineData.at(3));
		jobj.insert("password", "123456");
		jarr.append(jobj);
	}

	SHttpClient(URL("/api/privilege/batch_create")).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.json({ {"lists",jarr} })
		.success([=](const QByteArray& data)
			{
				QJsonParseError error;
				auto jdom = QJsonDocument::fromJson(data, &error);
				if (error.error != QJsonParseError::NoError && jdom["code"].toInt() > 1000) {
					return;
				}
				else {
					onSearch();
				}
			}).post();
}

void PManagerPage::readXLSXFile(const QString& filename)
{
	using namespace QXlsx;

	Document doc(filename);
	if (!doc.isLoadPackage()) {
		qWarning() << filename << "load failed！";
		return;
	}

	//Workbook* book =  doc.workbook();
	//qDebug() << book->sheetCount();
	//获取当前工作表，默认是第一个
	Worksheet* sheet =  doc.currentWorksheet();
	qDebug() << sheet->sheetName();

	//获取工作表大小
	CellRange range =  sheet->dimension();
	qDebug() << range.rowCount()<<range.columnCount();
	qDebug() << range.firstRow()<<range.firstColumn();
	qDebug() << range.lastRow()<<range.lastColumn();

	QJsonArray jarr;

	/*for (size_t r = 2; r <= range.rowCount(); r++)
	{
		QJsonObject jobj;
		for (size_t c = 1; c <= range.columnCount(); c++)
		{
			Cell *cell =  sheet->cellAt(r, c);
			if (cell)
			{
				auto v = cell->value();
				if (c == 1) 
					jobj.insert("user_id", v.toString());
				else if(c == 2)
					jobj.insert("username", v.toString());
				else if(c == 3)
					jobj.insert("gender", v.toString() == "未知" ? 0 : (v.toString() == "男" ? 1 : 2));
				else if(c == 4)
					jobj.insert("mobile", v.toString());
			}
		}
		jobj.insert("password", "123456");
		jarr.append(jobj);
	}*/

	for (size_t r = 2; r <= range.rowCount(); r++)
	{
		QJsonObject jobj;
		Cell* cell = nullptr;
		if (cell = sheet->cellAt(r, 1))
			jobj.insert("user_id", cell->value().toString());
		if(cell = sheet->cellAt(r, 2))
			jobj.insert("username", cell->value().toString());
		if(cell = sheet->cellAt(r, 3))
			jobj.insert("gender", cell->value() == "未知" ? 0 : (cell->value() == "男" ? 1 : 2));
		if(cell = sheet->cellAt(r, 4))
			jobj.insert("mobile", cell->value().toString());
		jarr.append(jobj);
	}

	SHttpClient(URL("/api/privilege/batch_create")).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.json({ {"lists",jarr} })
		.success([=](const QByteArray& data)
			{
				QJsonParseError error;
				auto jdom = QJsonDocument::fromJson(data, &error);
				if (error.error != QJsonParseError::NoError && jdom["code"].toInt() >= 1000) {
					return;
				}
				else {
					onSearch();
				}
			}).post();
}

void PManagerPage::writeCSVFile(const QString& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		qWarning() << filename << "open failed:" << file.errorString();
		return;
	}
	
	QTextStream stream(&file);
	//写表头
	for (size_t i = 1; i < m_model->columnCount() ; i++)
	{
		stream << m_model->horizontalHeaderItem(i)->text();
		if (i < m_model->columnCount() - 2)
			stream << ",";
	}
	stream << "\n";

	//写数据
	for (size_t r = 0; r < m_model->rowCount(); r++)
	{
		for (size_t c = 1; c < m_model->columnCount(); c++)
		{
			auto item = m_model->item(r, c);
			if (item)
			{
				stream << item->text();
			}
			if (c < m_model->columnCount() - 2) {
				stream << ",";
			}
		}
		stream << "\n";
	}
}

void PManagerPage::writeXLSXFile(const QString& filename)
{
	using namespace QXlsx;
	Document doc(filename);

	Worksheet* sheet =  doc.currentWorksheet();

	//写表头
	for (size_t i = 1; i < m_model->columnCount() ; i++)
	{
		sheet->write(1, i, m_model->horizontalHeaderItem(i)->text());
	}
	//写数据
	for (size_t r = 0; r < m_model->rowCount(); r++)
	{
		for (size_t c = 1; c < m_model->columnCount(); c++)
		{
			auto item = m_model->item(r, c);
			if (item)
			{
				sheet->write(r + 1 +1, c, item->text());
			}
		}
	}
	doc.save();
}
