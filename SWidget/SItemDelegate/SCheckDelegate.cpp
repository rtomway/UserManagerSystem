#include "SCheckDelegate.h"
#include <QStyleOption>
#include <QCheckBox>
#include <QApplication>
#include <QStyle>
#include<QMouseEvent>
#include<QPainter>

SCheckDelegate::SCheckDelegate(QObject* parent)
	:QStyledItemDelegate(parent)
{

}
void SCheckDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	
		// 获取选中状态
		bool checkState = index.data(Qt::UserRole).toBool();
	
		// 创建一个QStyleOptionButton对象
		QStyleOptionButton checkBoxStyle;

		// 设置复选框的位置和大小
		// 注意：这里假设你想要改变复选框的大小为20x20
	/*	int iconWidth = 20;
		int iconHeight = 20;
		checkBoxStyle.rect = QRect(option.rect.x(), option.rect.y()+10,
			iconWidth, iconHeight);*/

		painter->setRenderHint(QPainter::Antialiasing, true);

		// 设置复选框的状态
		checkBoxStyle.state = checkState ? QStyle::State_On : QStyle::State_Off;
		checkBoxStyle.state |= QStyle::State_Enabled;
		
		// 设置复选框的图标大小
	/*	checkBoxStyle.iconSize = QSize(iconWidth, iconHeight);*/

		QRect rect = QRect(option.rect.x(), option.rect.y() + 10, 20, 20);

		// 使用QApplication的当前风格来绘制复选框
		//QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxStyle, painter);
		 QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxStyle, painter);
		 if(!checkState)
			painter->drawPixmap(rect,QPixmap(":/ResourceClient/Uncheck.png"));
		 else
			painter->drawPixmap(rect, QPixmap(":/ResourceClient/checkAll.png"));
	}

bool SCheckDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	auto ev = dynamic_cast<QMouseEvent*>(event);
	if (event->type() == QEvent::MouseButtonPress && option.rect.contains(ev->position().toPoint()))
	{
		bool state = !index.data(Qt::UserRole).toBool();
		model->setData(index, state, Qt::UserRole);

	    if(state)
		{
			m_checkcount++;
		}
		else
		{
			m_checkcount--;
		}
		emit stateChanged(state, index);
	}
	return QStyledItemDelegate::editorEvent(event,model,option,index);
}
