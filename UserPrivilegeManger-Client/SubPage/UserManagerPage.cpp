#include "UserManagerPage.h"
#include "UserManagerPage.h"
#include "UserManagerPage.h"
#include "UserManagerPage.h"
#include "ui_UserManagerPage.h"
#include "SHttpClient.h"
#include "SApp.h"
#include "SHeaderView.h"
#include "SFieldTranslate.h"

#include "SSwitchButton.h"
#include "SCheckDelegate.h"
#include "SUrlDelegate.h"
#include "SSwitchDelegate.h"
#include "SButtonDelegate.h"

#include "UserDetailsDlg.h"
#include "UseraddDlg.h"
#include "SMaskWidget.h"

#include <set>
#include <QFile>
#include <QActionGroup>
#include <QFileDialog>

#include "xlsxdocument.h"   //文档
#include "xlsxworksheet.h"   //工作表
#include "xlsxworkbook.h"    //工作簿



UserManagerPage::UserManagerPage(QWidget* parent)
	:QWidget(parent)
	,ui(new Ui::UserManagerPage)
	,m_model(new QStandardItemModel(this))
	,m_setMenu(new QMenu(this))
{
	ui->setupUi(this);
	init();
	initMenu();
	initAddMenu();
	
}

UserManagerPage::~UserManagerPage()
{
	delete ui;
}

void UserManagerPage::init()
{
	QFile file(":/ResourceClient/style.css");
	if (file.open(QIODevice::ReadOnly))
	{
		setStyleSheet(file.readAll());
	}
	setAttribute(Qt::WA_StyledBackground);

	SFieldTrandlate::instance()->addTrans("user/user_id", "用户ID");
	SFieldTrandlate::instance()->addTrans("user/username", "用户名");
	SFieldTrandlate::instance()->addTrans("user/gender", "性别");
	SFieldTrandlate::instance()->addTrans("user/mobile", "电话");
	SFieldTrandlate::instance()->addTrans("user/email", "邮箱");
	SFieldTrandlate::instance()->addTrans("user/isEnable", "账号状态");

	ui->tableView->setModel(m_model);
	ui->tableView->setSelectionBehavior(QTableView::SelectRows);
	ui->tableView->setContentsMargins(0, 0, 0, 0);
	ui->tableView->setSelectionMode(QTableView::NoSelection);
	ui->tableView->setSelectionMode(QTableView::SingleSelection);
	
	ui->tableView->setFocusPolicy(Qt::NoFocus);
	ui->tableView->setEditTriggers(QTableView::NoEditTriggers);
	ui->tableView->setMouseTracking(true);

	ui->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	
	connect(ui->setBtn, &QPushButton::clicked, this, &UserManagerPage::onSet);
	
	
	onSearch();

	ui->setBtn->setFixedSize(32, 32);
	ui->setBtn->setIcon(QIcon(":/ResourceClient/set.png"));
	
	connect(ui->searchEdit, &QLineEdit::returnPressed, this, &UserManagerPage::onSearch);
	connect(ui->searchEdit, &QLineEdit::textChanged, [=](const QString&text)
		{
			if (text.isEmpty())
				onSearch();
		});
	connect(ui->searchBtn, &QPushButton::clicked, this, &UserManagerPage::onSearch);

	connect(ui->batchEnableBtn, &QPushButton::clicked, this, &UserManagerPage::onBatchEnable);
	connect(ui->batchDisableBtn, &QPushButton::clicked, this, &UserManagerPage::onBatchDisenable);
	connect(ui->batchDeleteBtn, &QPushButton::clicked, this, &UserManagerPage::onBatchDelete);

	connect(ui->exportBtn, &QPushButton::clicked,this, &UserManagerPage::onExport);

	//设置表格视图的头视图
	auto hHeaderView = new SHeaderView(Qt::Orientation::Horizontal);
	ui->tableView->setHorizontalHeader(hHeaderView);
	ui->tableView->setCornerButtonEnabled(false);
	//ui->tableView->verticalHeader()->setVisible(false);
	ui->tableView->verticalHeader()->setMaximumWidth(18);
	//复选框联动

	/*connect(m_model, &QStandardItemModel::itemChanged, [=](QStandardItem* item)
		{
			switch (item->checkState())
			{
			case Qt::Unchecked:
			{
				int total = 0;
				for (size_t i = 0; i < m_model->rowCount(); i++)
				{
					if (m_model->item(i, 0)->checkState())
					{
						total++;
					}
				}
				if (total > 0)
					hHeaderView->setState(Qt::PartiallyChecked);
				else
					hHeaderView->setState(Qt::Unchecked);
				break;
			}
			case Qt::Checked:
			{
				int total = 0;
				for (size_t i = 0; i < m_model->rowCount(); i++)
				{
					if (m_model->item(i, 0)->checkState())
					{
						total++;
					}
				}
				if (total == m_model->rowCount())
				{
					hHeaderView->setState(Qt::Checked);
				}
				else
				{
					hHeaderView->setState(Qt::PartiallyChecked);
				}
			}
			break;
			default:
				break;
			}
		});*/

	//item代理
	auto checkDelegate = new SCheckDelegate(ui->tableView);
	ui->tableView->setItemDelegateForColumn(0, checkDelegate);
	connect(hHeaderView, &SHeaderView::stateChanged, [=](int state)
		{
			for (size_t i = 0; i < m_model->rowCount(); i++)
			{
				auto item = m_model->item(i, 0);
				item->setData(state, Qt::UserRole);
			}
			if (state) {
				checkDelegate->setCheckCount(m_model->rowCount());
			}
			else {
				checkDelegate->setCheckCount(0);
			}
		});
	connect(checkDelegate, &SCheckDelegate::stateChanged, [=]()
		{
			size_t cnt = checkDelegate->checkCount();
			if (cnt == m_model->rowCount())
			{
				hHeaderView->setState(Qt::Checked);
			}
			else if (cnt > 0)
			{
				hHeaderView->setState(Qt::PartiallyChecked);
			}
			else
			{
				hHeaderView->setState(Qt::Unchecked);
			}
		});

	//SUrlDelegate
	auto urlDelegate = new SUrlDelegate(ui->tableView);
	ui->tableView->setItemDelegateForColumn(column("user_id"),urlDelegate);
	connect(urlDelegate, &SUrlDelegate::requestOpenUrl, [=](const QModelIndex& index)
		{
			if (!m_detailsDlg)
			{
				m_detailsDlg = new UserDetailsDlg(this);
				connect(m_detailsDlg, &UserDetailsDlg::userChanged, [=](const QJsonObject& user)
					{
						m_model->item(index.row(), column("user_id"))->setText(user.value("user_id").toString());
						m_model->item(index.row(), column("username"))->setText(user.value("username").toString());
						m_model->item(index.row(), column("gender"))->setText(user.value("gender").toInt() == 1 ? "男" : user.value("gender").toInt() == 2 ? "女" : "未知");
						m_model->item(index.row(), column("mobile"))->setText(user.value("mobile").toString());
						m_model->item(index.row(), column("email"))->setText(user.value("email").toString());
						m_model->item(index.row(), column("isEnable"))->setData(user.value("isEnable").toBool(),Qt::UserRole);
					});
			}
			//this->nativeParentWidget()->hide();
			connect(m_detailsDlg, &UserDetailsDlg::setshow, [=]
				{
					m_detailsDlg->hide();
					//onSearch();
					//this->nativeParentWidget()->show();
				});
			connect(m_detailsDlg, &UserDetailsDlg::userDeleted, [=](QJsonObject& user)
				{
					auto items = m_model->findItems(user.value("user_id").toString(), Qt::MatchFlag::MatchExactly);
					if (!items.isEmpty())
					{
						auto row = items.first()->row();
						m_model->removeRow(row);
					}
				});
			
			QJsonObject juser;
			juser.insert("user_id", m_model->item(index.row(), column("user_id"))->text());
			juser.insert("username", m_model->item(index.row(), column("username"))->text());
			juser.insert("gender", m_model->item(index.row(), column("gender"))->text());
			juser.insert("mobile", m_model->item(index.row(), column("mobile"))->text());
			juser.insert("email", m_model->item(index.row(), column("email"))->text());
			juser.insert("isEnable", m_model->item(index.row(), column("isEnable"))->data(Qt::UserRole).toBool()); 
			m_detailsDlg->setUser(juser);
			m_detailsDlg->resize(this->size());
			m_detailsDlg->show();
			
		});

	//SWitchDelegate
	auto switchDelegate = new SSwitchDelegate(ui->tableView);
	ui->tableView->setItemDelegateForColumn(column("isEnable"), switchDelegate);
	connect(switchDelegate, &SSwitchDelegate::stateChanged, [=](bool state,const QModelIndex& index)
		{
			auto user_id = m_model->item(index.row(), column("user_id"))->text();

			SHttpClient(URL("/api/user/alter?user_id="+user_id)).debug(true)
				.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
				.json({ {"isEnable",state} })
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

	//SButtonDelegate
	auto buttonDelegate = new SButtonDelegate(ui->tableView);
	ui->tableView->setItemDelegateForColumn(column("操作"), buttonDelegate);
	auto perimissionBtn = buttonDelegate->addButton(new QPushButton("权限设置"));
	auto delBtn=buttonDelegate->addButton(new QPushButton("删除"));
	QString styleStr = R"(
    QPushButton
	{
		background-color:rgb(97,162,243);
	}
	QPushButton:hover
	{
		background-color:red;
	}
		)";
	delBtn->setStyleSheet(styleStr);
	delBtn->setFixedSize(32,20);
	perimissionBtn->setStyleSheet(styleStr);
	perimissionBtn->setFixedSize(48, 20);
	connect(buttonDelegate, &SButtonDelegate::buttonClicked, [=](int id, const QModelIndex& index)
		{
			qDebug() << id << index;
			//删除用户
			if (id == 1)
			{
				auto user_id=m_model->item(index.row(), column("user_id"))->text();
				QJsonArray jarray;
				jarray.append(user_id);
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
							if (jdom["code"].toInt() < 1000)
							{
								m_model->removeRow(index.row());
							}
						})
						.post();
			}
			else if (id == 1)
			{

			}


		});
}

void UserManagerPage::initMenu()
{
	auto popPosionMenu = m_setMenu->addMenu("弹出位置");
	onPopUpPosition(popPosionMenu);
}
void UserManagerPage::initAddMenu()
{
	m_addMenu = new QMenu(ui->adduserBtn);
	ui->adduserBtn->setMenu(m_addMenu);
	m_addMenu->addAction("单条添加",this,&UserManagerPage::singleAdd);
	m_addMenu->addAction("批量添加",this,&UserManagerPage::batchAdd);
}

void UserManagerPage::onSearch()
{
	QVariantMap params;
	params.insert("isDeleted",false);
	auto filter = ui->searchEdit->text();
	if (!filter.isEmpty())
	{
		params.insert("query", filter); 
	}
	SHttpClient(URL("/api/user/list")).debug(true)
		.header("Authorization","Bearer" + sApp->userData("user/token").toString())
		.params(params)
		.success([=](const QByteArray& data)
			{
				QJsonParseError error;
				auto jdom = QJsonDocument::fromJson(data, &error);
				if (error.error != QJsonParseError::NoError)
				{
					qWarning() << "失败";
					return;
				}
				parseJson(jdom.object());
			}).get();
}

void UserManagerPage::setBatchEnable(bool enable)
{
	QJsonObject jobj;
	QJsonArray jarray;
	for (size_t i = 0; i < m_model->rowCount(); i++)
	{
		auto item = m_model->item(i);
		if (item && item->data(Qt::UserRole).toBool())
		{
			auto isEnable = m_model->item(i, column("isEnable"))->data(Qt::UserRole);
			auto user_id = m_model->item(i, column("user_id"))->text();
			if ( isEnable!= enable)
			{
				QJsonObject rObj;
				QJsonObject filterObj;
				filterObj.insert("user_id",user_id);
				QJsonObject updateObj;
				updateObj.insert("isEnable", enable);
				rObj.insert("filter", filterObj);
				rObj.insert("update", updateObj);
				jarray.append(rObj);

				//模型实时更新
				m_model->item(i, column("isEnable"))->setData(enable, Qt::UserRole);
			}
		}
	}
	jobj.insert("lists", jarray);

	SHttpClient(URL("/api/user/batch_alter")).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.json(jobj)
		.fail([](const QString& msg, int code)
			{

			})
		.success([](const QByteArray& data)
			{

			})
			.post();
}

void UserManagerPage::onBatchEnable()
{
	setBatchEnable(true);
}

void UserManagerPage::onBatchDisenable()
{
	setBatchEnable(false);
}

void UserManagerPage::onBatchDelete()
{
	QJsonObject jobj;
	QJsonArray jarray;
	std::set<int, std::greater<int>> deleteRows;
	for (size_t i = 0; i < m_model->rowCount(); i++)
	{
		auto item = m_model->item(i);
		qDebug() << item  << item->data(Qt::UserRole);
		if (item && item->data(Qt::UserRole).toBool())// == Qt::CheckState::Checked)
		{
			jarray.append(m_model->item(i,column("user_id"))->text());
			deleteRows.insert(i);
		}
	}
	jobj.insert("lists", jarray);

	SHttpClient(URL("/api/user/delete")).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.json(jobj)
		.fail([](const QString& msg, int code)
			{

			})
		.success([=](const QByteArray& data)
			{
				for (auto row : deleteRows)
				{
					m_model->removeRow(row);
				}
			})
		.post();
}

void UserManagerPage::onSet()
{
	m_setMenu->move(ui->setBtn->mapToGlobal(QPoint(32, 32)));
	m_setMenu->exec();
}

void UserManagerPage::onPopUpPosition(QMenu* popPosionMenu)
{
	auto left = popPosionMenu->addAction("左侧");
	auto middle = popPosionMenu->addAction("中间");
	auto right = popPosionMenu->addAction("右侧");

	left->setIconVisibleInMenu(true);
	middle->setIconVisibleInMenu(true);
	right->setIconVisibleInMenu(true);

	QActionGroup* actionGroup = new QActionGroup(popPosionMenu);
	actionGroup->addAction(left);
	actionGroup->addAction(middle);
	actionGroup->addAction(right);
	middle->setIcon(QIcon(":/ResourceClient/right.png"));
	actionGroup->setExclusive(true);

	connect(left, &QAction::triggered, [=]
		{
			SMaskWidget::instance()->setPopPostion(SMaskWidget::LeftWidget);
			left->setIcon(QIcon(":/ResourceClient/right.png"));
			middle->setIcon(QIcon());
			right->setIcon(QIcon());
		});
	connect(middle, &QAction::triggered, [=]
		{
			SMaskWidget::instance()->setPopPostion(SMaskWidget::MiddleWidget);
			middle->setIcon(QIcon(":/ResourceClient/right.png"));
			left->setIcon(QIcon());
			right->setIcon(QIcon());
		});
	connect(right, &QAction::triggered, [=]
		{
			SMaskWidget::instance()->setPopPostion(SMaskWidget::RightWidget);
			right->setIcon(QIcon(":/ResourceClient/right.png"));
			middle->setIcon(QIcon());
			left->setIcon(QIcon());
		});

}

void UserManagerPage::parseJson(const QJsonObject& obj)
{
	m_model->clear();

	//QStringList headers = {"", "用户ID","用户名","性别","电话","邮箱","帐号状态" };
		//m_model->setHorizontalHeaderLabels(headers);
	for (size_t i = 0; i < m_fieldName.size(); i++)
	{
		m_model->setHorizontalHeaderItem
		(i, new QStandardItem(SFieldTrandlate::instance()->trans("user", m_fieldName[i])));
	}
	auto jarray = obj.value("data").toObject().value("lists").toArray();
	for (size_t i = 0; i < jarray.size(); i++)
	{
		auto juser = jarray.at(i).toObject();
		m_model->appendRow(ItemsFromJsonObject(juser));
	}
		//表格
		
		//QStandardItem* item = nullptr;
		//item = new QStandardItem;
		//item->setCheckable(true);
		//items.append(item);
		////auto item = new QStandardItem(juser.value("id").toVariant().toString());
		//item = new QStandardItem(juser.value("user_id").toVariant().toString());
		//item->setTextAlignment(Qt::AlignCenter);
		//items.append(item);
		//items.append(new QStandardItem(juser.value("username").toVariant().toString()));
		//items.append(new QStandardItem(gender==0?"未知":(gender==1?"男":"女")));
		//items.append(new QStandardItem(juser.value("mobile").toVariant().toString()));
		//items.append(new QStandardItem(juser.value("email").toVariant().toString()));
		//item = new QStandardItem(isEnable ? "正常":"禁用");
		//item->setData(isEnable, Qt::UserRole);
		//items.append(item);
	ui->tableView->setColumnWidth(0, 26);
	ontableviewResize();
	dynamic_cast<SHeaderView*>(ui->tableView->horizontalHeader())->setState(Qt::Unchecked);
}

void UserManagerPage::ontableviewResize()
{
	int columnCount = m_model->columnCount();
	int totalWidth = ui->tableView->width() - 20-18;
	if (columnCount > 0) {
		int columnWidth = totalWidth / (columnCount - 1);
		for (int i = 0; i < columnCount; i++) {
			ui->tableView->setColumnWidth(i, columnWidth);
			ui->tableView->setRowHeight(i, 40);
		}
	}
	ui->tableView->setColumnWidth(0, 20);
}

int UserManagerPage::column(const QString& fieldName)
{
	auto pos = m_fieldName.indexOf(fieldName);
	if (pos == -1)
		qWarning() << "fieldName" << fieldName << "not exists";
	return pos;
}

QList<QStandardItem*> UserManagerPage::ItemsFromJsonObject(const QJsonObject& juser)
{
	auto gender = juser.value("gender").toInteger();
	auto isEnable = juser.value("isEnable").toInt();
		
	QList<QStandardItem*>items;
	//for (auto it = juser.begin(); it != juser.end(); it++)
	for (const auto& field : m_fieldName)
	{
		auto item = new QStandardItem;
		item->setTextAlignment(Qt::AlignCenter);
		if (field == "gender")
		{
			item->setText(gender == 0 ? "未知" : (gender == 1 ? "男" : "女"));
		}
		else if (field == "isEnable")
		{
			item->setText(isEnable ? "正常" : "禁用");
			item->setData(isEnable, Qt::UserRole);
		}
		else
		{
			item->setText(juser.value(field).toVariant().toString());   //若int无法转string   先variant
		}
		items.append(item);
	}
	return items;
}

void UserManagerPage::resizeEvent(QResizeEvent* ev)
{
	if (m_detailsDlg)
		m_detailsDlg->resize(this->size());

	ontableviewResize();
	
}

void UserManagerPage::singleAdd()
{
	if (!m_userAddDlg)
	{
		m_userAddDlg = new UseraddDlg;
		connect(m_userAddDlg, &UseraddDlg::newUser, [=](const QJsonObject& juser)
			{
				m_model->insertRow(0, ItemsFromJsonObject(juser));
				m_userAddDlg->close();
			});
	}
	//m_userAddDlg->show();
	SMaskWidget::instance()->popUp(m_userAddDlg);
}

void UserManagerPage::batchAdd()
{
	auto filename = QFileDialog::getOpenFileName(this, "选择用户表格", "./", "xlsx (*.xlsx;*.csv);;all (*.*)");
	if (filename.isEmpty())
	{
		return;
	}
	//判断文件类型
	auto extraName = QFileInfo(filename).suffix();
	if (extraName.compare("csv", Qt::CaseInsensitive) == 0)
	{
		readCsv(filename);
	}
	else if (extraName.compare("xlsx", Qt::CaseInsensitive) == 0)
	{
		readXlsx(filename);
	}
}

void UserManagerPage::readCsv(const QString& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << filename << "open failed:" << file.errorString();
		return;
	}
	QTextStream stream(&file);
	auto guessBom = file.read(3);
	if (guessBom.startsWith("\xEF\xBB\xBf"))
	{
		stream.setEncoding(QStringConverter::Utf8);
	}
	else
	{
		stream.setEncoding(QStringConverter::System);
	}
	
	auto headers = stream.readLine().split(',');

	QJsonArray jarray;
	while (!stream.atEnd())
	{
		QJsonObject obj;
		auto lineData = stream.readLine().split(',');
		if (lineData.size() != 4)
		{
			qWarning() << "字段数不符，导入失败";
			return;
		}
		obj.insert("user_id", lineData.at(0));
		obj.insert("username", lineData.at(1));
		obj.insert("gender", lineData.at(2)=="男"?1: lineData.at(2) == "女"?2:0);
		obj.insert("mobile", lineData.at(3));
		jarray.append(obj);
	}
	SHttpClient(URL("/api/user/batch_create")).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.json({ {"list",jarray } })
		.success([=](const QByteArray& data)
			{
				QJsonParseError error;
				auto jdom = QJsonDocument::fromJson(data, &error);
				if ((error.error != QJsonParseError::NoError) && jdom["code"].toInt() > 1000)
				{
					return;
				}
				else
				{
					onSearch();
				}
			}).post();
}

void UserManagerPage::readXlsx(const QString& filename)
{
	using namespace QXlsx;

	Document doc(filename);
	if (!doc.load())
	{
		qWarning() << filename << "xlsx load failed";
	}

	//获取工作簿
	Workbook* book = doc.workbook();
	qDebug() << "sheetCount" << book->sheetCount();
	qDebug() << "sheetName" << doc.sheetNames();

	//获取工作表
	//book->sheet(0);
	Worksheet* sheet = doc.currentWorksheet();

	//获取单元格
	CellRange range = sheet->dimension();
	QJsonArray jarray;
	for (size_t r = 2; r <= range.rowCount(); r++)
	{
		QJsonObject obj;
		for (size_t c = 1; c <= range.columnCount();c++)
		{
			Cell* cell = sheet->cellAt(r, c);
			if (cell)
			{
				auto v= cell->value().toString();
				if (c == 1)
					obj.insert("user_id", v);
				else if (c == 2)
					obj.insert("username", v);
				else if (c == 3)
					obj.insert("gender", v == "男" ? 1 : (v == "女" ? 2 : 0));
				else if (c == 4)
					obj.insert("mobile", v);
			}
		}
		jarray.append(obj);
		qDebug() << obj;
	}
	SHttpClient(URL("/api/user/batch_create")).debug(true)
		.header("Authorization", "Bearer" + sApp->userData("user/token").toString())
		.json({ {"list",jarray } })
		.success([=](const QByteArray& data)
			{
				QJsonParseError error;
				auto jdom = QJsonDocument::fromJson(data, &error);
				if ((error.error != QJsonParseError::NoError) && jdom["code"].toInt() > 1000)
				{
					return;
				}
				else
				{
					onSearch();
				}
			}).post();
}

void UserManagerPage::writeCsv(const QString& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly))
	{
		qWarning() << filename << "open failed" << file.errorString();
		return;
	}
	QTextStream stream(&file);
	//写表头
	for (size_t i = 1; i < m_model->columnCount() - 1; i++)
	{
		stream << m_model->horizontalHeaderItem(i)->text();
		if (i < m_model->columnCount() - 2)
		{
			stream << ",";
		}
	}
	stream << "\n";
	//写数据
	for (int r = 0; r < m_model->rowCount(); r++)
	{
		for (int c = 1; c < m_model->columnCount() - 1; c++)
		{
			auto item = m_model->item(r, c);
			if (item)
				stream << item->text();
			if (c < m_model->columnCount() - 2)
				stream << ",";
		}
		stream << "\n";
	}
}

void UserManagerPage::writeXlsx(const QString& filename)
{
	using namespace QXlsx;
	Document doc(filename);
	Worksheet* sheet = doc.currentWorksheet();
	//写表头
	for (size_t i = 1; i < m_model->columnCount()-1; i++)
	{
		sheet->write(1,i,m_model->horizontalHeaderItem(i)->text());
	}
	//写数据
	for (size_t r = 0; r < m_model->rowCount(); r++)
	{
		for (size_t c = 1; c < m_model->columnCount() - 1; c++)
		{
			auto item = m_model->item(r, c);
			if (item)
			{
				if (c == column("isEnable"))
				{
					sheet->write(r + 2, c, item->data(Qt::UserRole).toBool());
				}
				else
					sheet->write(r + 2, c, item->text());
			}
		}
	}
	doc.save();
}

void UserManagerPage::onExport()
{
	auto filename= QFileDialog::getSaveFileName(this, "选择目录", "./undefined.xlsx", "xlsx (*.xlsx);;csv (*.csv);;all (*.*)");
	if (filename.isEmpty())
		return;

	auto extra = QFileInfo(filename).suffix();
	if (extra.compare("csv", Qt::CaseInsensitive) == 0) {
		writeCsv(filename);
	}
	else if (extra.compare("xlsx", Qt::CaseInsensitive) == 0) {
		writeXlsx(filename);
	}
	else
	{
		qWarning() << filename << "文件格式错误";
		return;
	}

}
