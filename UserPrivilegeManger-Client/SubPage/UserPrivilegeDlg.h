#ifndef  USERPRIVILEGEDLG_H_
#define  USERPRIVILEGEDLG_H_

#include <QJsonObject>
#include <QStandardItemModel>
#include <QWidget>

class QPushButton;
class QLabel;
class QRadioButton;
class QLineEdit;
class QTableView;

class UserPrivilegeDlg : public QWidget
{
	Q_OBJECT
public:
	UserPrivilegeDlg(QWidget* parent = nullptr);
	void init();

	void setUser(const QJsonObject& user);
	void getAllPriviegeList();
protected:
	void parseJson(const QJsonObject& obj);
	QList<QStandardItem*> ItemsFromJsonObject(const QJsonObject& jobj);
private:
	QJsonObject m_juser;

    QLineEdit* m_user_id_edit{};
    QLineEdit* m_privilege_search_edit{};
    QStandardItemModel* m_selectModel{};
	QTableView* m_tableView{};


	QStringList m_fieldName = { "","name","descr"};
};

#endif //! USERDETAILSDLG_H_