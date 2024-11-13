#ifndef USERMANAGERPAGE_H_
#define USERMANAGERPAGE_H_

#include<QWidget>
#include<QStandardItemModel>
#include "UserDetailsDlg.h"
#include "UseraddDlg.h"
#include <QMenu>

namespace Ui { class UserManagerPage; }

class UserManagerPage :public QWidget
{
	Q_OBJECT
public:
	UserManagerPage(QWidget* parent = nullptr);
	~UserManagerPage();
	void init();
	void onSearch();
	void setBatchEnable(bool enable);
	void onBatchEnable();
	void onBatchDisenable();
	void onBatchDelete();
	void onSet();
	void initMenu();
public:
	void onPopUpPosition(QMenu*menu);
private:
	void parseJson(const QJsonObject& obj);
	void ontableviewResize();
private:
	Ui::UserManagerPage* ui{};
	QStandardItemModel* m_model{};

	QStringList m_fieldName = { "","user_id","username","gender","mobile","email","isEnable" ,"操作" };
	int column(const QString& fieldName);
	QList<QStandardItem*>ItemsFromJsonObject(const QJsonObject& juser);

	UserDetailsDlg* m_detailsDlg{};
	UseraddDlg* m_userAddDlg{};

	QMenu* m_setMenu{};
	QMenu* m_addMenu{};
private:
	void initAddMenu();
	void singleAdd();
	void batchAdd();

	void readCsv(const QString& filename);
	void readXlsx(const QString& filename);

	void writeCsv(const QString& filename);
	void writeXlsx(const QString& filename);
	void onExport();
protected:
	void resizeEvent(QResizeEvent* ev)override;
};

#endif // !USERMANAGERPAGE_H_
