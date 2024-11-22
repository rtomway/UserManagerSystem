#ifndef USERMANAGERPAGE_H_
#define USERMANAGERPAGE_H_

#include<QWidget>
#include<QStandardItemModel>
#include "PersonMessage.h"
#include "UseraddDlg.h"
#include <QMenu>
#include "UserDetailDlg.h"
#include "SHeaderView.h"
#include "SSwitchDelegate.h"

namespace Ui { class UserManagerPage; }

class UserManagerPage :public QWidget
{
	Q_OBJECT
public:
	UserManagerPage(QWidget* parent = nullptr);
	~UserManagerPage();
	void init();
	void onSearch();
	void GetUserGrade(int grade);
	void showUserDetail(const QModelIndex& index);
	void setBatchEnable(bool enable);
	void onBatchEnable();
	void onBatchDisenable();
	void onBatchDelete();
	void onSet();
	void initMenu();
public:
	void onPopUpPosition(QMenu*menu);
	QJsonObject rowMessage(const QString& user_id);
private:
	int m_userGrade;
	void parseJson(const QJsonObject& obj);
	void ontableviewResize();

private:
	Ui::UserManagerPage* ui{};
	QStandardItemModel* m_model{};

	QStringList m_fieldName= { " ","user_id","username","gender","mobile","email","isEnable","操作" };
	int column(const QString& fieldName);
	QList<QStandardItem*>ItemsFromJsonObject(const QJsonObject& juser);

	UserDetailDlg* m_detailsDlg{};
	UseraddDlg* m_userAddDlg{};

	QMenu* m_setMenu{};
	QMenu* m_addMenu{};
	SSwitchDelegate*  switchDelegate;
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
