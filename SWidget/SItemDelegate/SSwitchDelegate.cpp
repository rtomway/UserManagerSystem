﻿#include "SSwitchDelegate.h"
#include <QStyleOption>
#include <QCheckBox>
#include <QApplication>
#include <QStyle>
#include <QMouseEvent>
#include <QPainter>
#include <QAbstractItemView>
#include <QMessageBox>

#define BUTTON_WIDTH  45
#define BUTTON_HEIGHT 20

SSwitchDelegate::SSwitchDelegate(QObject* parent)
	:QStyledItemDelegate(parent)
{
}

void SSwitchDelegate::GetsUserGrade(int grade) 
{
	m_Grade = grade;
}
void SSwitchDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	painter->save();
	painter->setRenderHint(QPainter::RenderHint::Antialiasing);

	auto state = index.data(Qt::UserRole).toBool();
	QString text = state ? "正常" : "禁用";

	//文字大小
	int tw = painter->fontMetrics().horizontalAdvance(text);
	int th = painter->fontMetrics().height();

	//文本+按钮整体居中计算
	int hspace = (option.rect.width() - (tw + BUTTON_WIDTH)) / 2;
	int vspace = (option.rect.height() - th) / 2;
	//绘制文本
	painter->drawText(QPoint(option.rect.x() + hspace, option.rect.bottom() - vspace - 2), text);

	//绘制按钮
	QRect btnRect = { option.rect.x() + hspace + tw + 10 ,option.rect.y() + (option.rect.height() - BUTTON_HEIGHT) / 2,BUTTON_WIDTH,BUTTON_HEIGHT };

	painter->setBrush(state ? QColor(97, 162, 243) : Qt::gray);
	painter->setPen(Qt::gray);
	painter->drawRoundedRect(btnRect, btnRect.height() / 2, btnRect.height() / 2);

	//小球
	if (option.state & QStyle::State_MouseOver && btnRect.contains(m_mousePos)) {
		painter->setBrush(QColor(232, 232, 232));
	}
	else {
		painter->setBrush(Qt::white);
	}
	painter->setPen(Qt::NoPen);
	if (state)
		painter->drawEllipse(btnRect.right() - 20, btnRect.y(), 20, 20);
	else
		painter->drawEllipse(btnRect.x(), btnRect.y(), 20, 20);

	painter->restore();
}

bool SSwitchDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	if (event->type() == QEvent::MouseButtonRelease) {
		if (!m_Grade)
		{
			QMessageBox msgBox;
			msgBox.setText("无权限");
			msgBox.setWindowTitle("提示");
			msgBox.setIcon(QMessageBox::Information);
			msgBox.setStyleSheet(R"(
				QMessageBox {	
						background-color:rgb(149, 237, 208);
						border-radius: 3px;
						color: red;
					}
			QMessageBox QLabel {
                   font-family: '华文行楷';
                   font-size: 20px;
                   color: #FF9933;
                   padding-left: 0px;
                   padding-top: 20px;
                   margin-right: 10px;
                   text-align: center;
						}
					QMessageBox QPushButton {
                         background-color: #99FF33;
                         color: black;
                         width: 40px;
                         height: 20px;
                         margin-right: 10px;
                         border: none;
                         border-radius: 4px;
									}
				QMessageBox QPushButton:hover {
					    background-color: #CCFF33;
										}
								)");
			msgBox.exec();
			return QStyledItemDelegate::editorEvent(event, model, option, index);
		}
		auto state = model->data(index, Qt::UserRole).toBool();
		model->setData(index, !state, Qt::UserRole);
		emit stateChanged(!state, index);
	}
	else if (event->type() == QEvent::MouseMove)
	{
		auto ev = dynamic_cast<QMouseEvent*>(event);
		m_mousePos = ev->position().toPoint();

		auto view = dynamic_cast<QAbstractItemView*>(parent());
		if (view) {
			view->viewport()->update(option.rect);
		}
	}

	return QStyledItemDelegate::editorEvent(event, model, option, index);
}

