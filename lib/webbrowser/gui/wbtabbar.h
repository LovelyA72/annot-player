#ifndef WBTABBAR_H
#define WBTABBAR_H

// gui/wbtabbar.h
// 4/24/2012

#include <QtGui/QMouseEvent>
#include <QtGui/QTabBar>

class WbTabBar : public QTabBar
{
  Q_OBJECT
  Q_DISABLE_COPY(WbTabBar)
  typedef WbTabBar Self;
  typedef QTabBar Base;

public:
  explicit WbTabBar(QWidget *parent = nullptr)
    : Base(parent) { }

signals:
  void doubleClicked(int index);

  // - Events -
protected:
  void mouseDoubleClickEvent(QMouseEvent *e) override
  {
    if (e->button() == Qt::LeftButton && !e->modifiers()) {
      emit doubleClicked(tabAt(e->globalPos()));
      e->accept();
    } else
      Base::mouseDoubleClickEvent(e);
  }
};

#endif // WBTABBAR_H
