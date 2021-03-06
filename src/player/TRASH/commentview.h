#ifndef COMMENTVIEW_H
#define COMMENTVIEW_H

// commentview.h
// 10/9/2011

#include "module/qtext/webdialog.h"

typedef QxWebDialog CommentViewBase;

class CommentView: public CommentViewBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CommentView)
  typedef CommentView Self;
  typedef WebDialog Base;

  qint64 tokenId_;

  // - Constructions -
public:
  explicit
  CommentView(QWidget *parent = nullptr);

  // - Properties -
public:
  qint64 tokenId() const;
public slots:
  void setTokenId(qint64 mtid);

public:
  static QUrl urlForTokenId(qint64 mtid);

  // - Events -
public:
  void setVisible(bool visible) override;
};

#endif // COMMENTVIEW_H
