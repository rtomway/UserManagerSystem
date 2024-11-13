﻿#include "SEventfilterObject2.h"
#include <QMouseEvent>
#include <QWidget>
#include <QGraphicsDropShadowEffect>
SEventFilterObject2::SEventFilterObject2(QObject *parent) 
    : QObject(parent)
{
    //如果指定了窗口为父对象，则设置窗口阴影
    auto *w = dynamic_cast<QWidget*>(parent);
    if (w)
    {
       /* auto* effect = new QGraphicsDropShadowEffect(this);
        effect->setOffset(0);
        effect->setBlurRadius(10);
        effect->setColor(QColor(60, 60, 60));
        effect->setColor(QColor(150,150,150));
        w->setGraphicsEffect(effect);*/
    }
}

bool SEventFilterObject2::eventFilter(QObject *watched, QEvent *ev)
{
    auto* mev = static_cast<QMouseEvent*>(ev);
    if(ev->type() == QEvent::MouseButtonPress)
    {
        m_pressPos = mev->pos();
    }
	else if (ev->type() == QEvent::MouseMove)
    {
        //鼠标左键按着移动，就移动窗口，并返回true表示事件已经处理了
        if (mev->buttons() & Qt::LeftButton)
        {
            auto w = dynamic_cast<QWidget*>(watched);
			//1,相对于窗口，如果是顶层窗口，是没毛病的
			//w->move(mev->globalPosition().toPoint() - m_pressPos);
			//2,相对于父窗口，主窗口上的悬浮球必须相对父窗口移动
			w->move(w->mapToParent(mev->pos()) - m_pressPos);
			m_isMoving = true;
			return true;
		}
	}
	else if (ev->type() == QEvent::MouseButtonRelease)
	{
        //如果是移动窗口，则取消移动，并返回true，表示事件已经处理了
        if (mev->button() == Qt::LeftButton && m_isMoving)
        {
            m_isMoving = false;
            m_pressPos = QPoint();
            return true;
        }
    }
    return false;
}
