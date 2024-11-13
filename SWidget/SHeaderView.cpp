#include "SHeaderView.h"
#include <QPainter>
#include <QMouseEvent>
#include <QCheckBox>
#define CHECK_BOX_COLUMN 0

SHeaderView::SHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent),
      m_bPressed(false),
      m_bChecked(false),
      m_bTristate(false),
      m_bNoChange(false),
	  m_bHover(false)
{
    // setStretchLastSection(true);
    setHighlightSections(false);
    setMouseTracking(true);
    
    // 响应鼠标
    setSectionsClickable(true);
    setStyleSheet(R"(QHeaderView::section{background-color: rgb(242, 255, 251);})");
    setAttribute(Qt::WA_StyledBackground);
    
}

// 槽函数，用于更新复选框状态
void SHeaderView::setState(int state)
{
    if (state == Qt::PartiallyChecked) {
        m_bTristate = true;
        m_bNoChange = true;
    } else {
        m_bNoChange = false;
    }

    m_bChecked = (state != Qt::Unchecked);
	viewport()->update();
}

// 绘制复选框
void SHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
  
    QHeaderView::paintSection(painter, rect, logicalIndex);
   
    painter->restore();
    painter->setRenderHint(QPainter::Antialiasing);
    if (logicalIndex == CHECK_BOX_COLUMN)
    {
        QPixmap pixmap;
        if (m_bTristate && m_bNoChange) {
            pixmap = QPixmap(":/ResourceClient/checksome.png");
        }
        else if (m_bChecked) {
            pixmap = QPixmap(":/ResourceClient/checkAll.png");
        }
        else {
            pixmap = QPixmap(":/ResourceClient/Uncheck.png");
        }
        
        painter->drawPixmap(QRect(rect.x()-2,rect.y(),rect.width(),rect.height()), pixmap);
      
    }
}

// 鼠标按下表头
void SHeaderView::mousePressEvent(QMouseEvent *event)
{
    int nColumn = logicalIndexAt(event->pos());
    if ((event->buttons() & Qt::LeftButton) && (nColumn == CHECK_BOX_COLUMN))
    {
        m_bPressed = true;
    }
    else
    {
        QHeaderView::mousePressEvent(event);
    }
}

// 鼠标从表头释放，发送信号，更新model数据
void SHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_bPressed)
    {
        if (m_bTristate && m_bNoChange)
        {
            m_bChecked = true;
            m_bNoChange = false;
        }
        else
        {
            m_bChecked = !m_bChecked;
        }

        viewport()->update();

        Qt::CheckState state = m_bChecked ? Qt::Checked : Qt::Unchecked;

        emit stateChanged(state);
    }
    else
    {
        QHeaderView::mouseReleaseEvent(event);
    }

    m_bPressed = false;
}

// 鼠标滑过、离开，更新复选框状态
bool SHeaderView::event(QEvent *event)
{
    updateSection(0);
    if (event->type() == QEvent::Enter)
    {
		QEnterEvent* pEvent = dynamic_cast<QEnterEvent*>(event);
        int nColumn = logicalIndexAt(pEvent->position().toPoint());
        if (nColumn == CHECK_BOX_COLUMN)
        {
            m_bHover = true;

            return true;
        }
    }
    else if (event->type() == QEvent::Leave)
    {
        m_bHover = false;
        return true;
    }

    return QHeaderView::event(event);
}


