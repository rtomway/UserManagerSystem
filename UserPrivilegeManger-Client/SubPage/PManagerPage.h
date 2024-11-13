#ifndef  PMANAGERPAGE_H_
#define  PMANAGERPAGE_H_

#include <QWidget>
#include <QStandardItemModel>
#include <QMenu>

namespace Ui { class PManagerPage; }
class PAddEditDlg;

class PManagerPage : public QWidget
{
	Q_OBJECT
public:
	PManagerPage(QWidget* parent = nullptr);
	~PManagerPage();
	void init();

public:
	void onSearch();
	void onExport();

	void onBatchDelete();
private:
	void parseJson(const QJsonObject& obj);
private:
	Ui::PManagerPage* ui{};
	QStandardItemModel* m_model{};
	QStringList m_fieldName = {"user_id","username","privilege_read","privilege_edit","privilege_add","privilege_delete"};
	int column(const QString& field);
	QList<QStandardItem*> ItemsFromJsonObject(const QJsonObject& jobj);

	PAddEditDlg* m_pAddEditDlg{};
	QMenu* m_importMenu{};
	void initMenu();
	void singleImport();
	void batchImport();

	void readCSVFile(const QString& filename);
	void readXLSXFile(const QString& filename);


	void writeCSVFile(const QString& filename);
	void writeXLSXFile(const QString& filename);
};

#endif //!  USERMANAGERPAGE_H_