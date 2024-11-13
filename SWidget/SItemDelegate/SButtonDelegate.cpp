#include "SButtonDelegate.h"
#include <QStyleOption>
#include <QCheckBox>
#include <QApplication>
#include <QStyle>
#include <QMouseEvent>
#include <QPainter>
#include <QAbstractItemView>

#define	BUTTON_HEIGHT(id)  m_buttons.at(id)->height()
#define	BUTTON_WIDTH(id)  m_buttons.at(id)->width()

SButtonDelegate::SButtonDelegate(QObject* parent)
	:QStyledItemDelegate(parent)
{

}

QAbstractButton* SButtonDelegate::addButton(QAbstractButton* button)
{
	m_buttons.emplace_back(button);

	return button;
}

QAbstractButton* SButtonDelegate::button(int id)
{
	Q_ASSERT(id >= 0 && m_buttons.size());
	return m_buttons.at(id).get();
}

void SButtonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	painter->save();
	//按钮总宽度
	auto totalWidth = buttonTotalWidth();
	int hspace = (option.rect.width() - totalWidth - m_spacing * (m_buttons.size() - 1)) / 2;
	int vspace = (option.rect.height()-BUTTON_HEIGHT(0)) / 2;

	for (size_t i = 0; i < m_buttons.size(); i++)
	{
		QStyleOptionButton button;
		button.rect = QRect(option.rect.x() + hspace + i * m_spacing + buttonTotalWidth(i), option.rect.y() + vspace,
			BUTTON_WIDTH(i), BUTTON_HEIGHT(i));
		button.state |= QStyle::State_Enabled;
		if (option.state & QStyle::State_MouseOver&&button.rect.contains(m_mousePos))
			button.state |= QStyle::State_MouseOver;

		auto btn = m_buttons.at(i).get();
		button.text = btn->text();
		btn->style()->drawControl(QStyle::CE_PushButton, &button, painter, btn);
	}

	painter->restore();
}

bool SButtonDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	auto ev = dynamic_cast<QMouseEvent*>(event);
	if (event->type() == QEvent::MouseMove)
	{
		m_mousePos = ev->position().toPoint();

		auto view = dynamic_cast<QAbstractItemView*>(parent());
		if (view)
			view->viewport()->update(option.rect);
	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		auto totalWidth = buttonTotalWidth();
		int hspace = (option.rect.width() - totalWidth - m_spacing * (m_buttons.size() - 1)) / 2;
		int vspace = (option.rect.height() - BUTTON_HEIGHT(0)) / 2;

		for (size_t i = 0; i < m_buttons.size(); i++)
		{
			QRect rect(option.rect.x() + hspace + i * m_spacing + buttonTotalWidth(i), option.rect.y() + vspace,
				BUTTON_WIDTH(i), BUTTON_HEIGHT(i));

			if (rect.contains(ev->position().toPoint())) {
				emit buttonClicked(i, index);
				break;
			}
		}

	}
	return QStyledItemDelegate::editorEvent(event, model, option,index);
}


int SButtonDelegate::buttonTotalWidth(size_t count) const
{
	size_t cnt = count;
	if (count == -1)
		cnt = m_buttons.size();
	size_t w = 0;
	for (size_t i = 0; i < cnt; i++)
	{
		w += m_buttons.at(i)->width();
	}
	return w;
}
