#ifndef SCHECKDELEGATE_H_
#define  SCHECKDELEGATE_H_

#include<QStyledItemDelegate>

class SCheckDelegate :public QStyledItemDelegate
{
	Q_OBJECT
public:
	SCheckDelegate(QObject* parent = nullptr)
		:QStyledItemDelegate(parent)
	{

	}
	size_t checkCount()const { return m_checkcount; }
	void setCheckCount(size_t count) { m_checkcount = count; }
protected:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
signals:
	void stateChanged(int state, const QModelIndex& index);
private:
	size_t m_checkcount{};
};

#endif // SCHECKDELEGATE_H_
