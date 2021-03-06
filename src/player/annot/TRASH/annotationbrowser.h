#ifndef ANNOTATIONBROWSER_H
#define ANNOTATIONBROWSER_H

// annotationbrowser.h
// 10/23/2011

#include "src/common/acmainwindow.h"
#include "module/annotcloud/annotation.h"
#include <QtCore/QModelIndex>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QStandardItemModel;
class QSortFilterProxyModel;
class QToolButton;
QT_END_NAMESPACE

class AcFilteredTableView;
class AnnotationEditor;
class SignalHub;

class AnnotationBrowser : public AcMainWindow
{
  Q_OBJECT
  Q_DISABLE_COPY(AnnotationBrowser)
  typedef AnnotationBrowser Self;
  typedef AcMainWindow Base;

  typedef AnnotCloud::Annotation Annotation;
  typedef AnnotCloud::AnnotationList AnnotationList;

protected:
  enum HeaderData {
    HD_Text = 0,
    HD_Pos,
    HD_UserAlias,
    HD_CreateTime,
    HD_UpdateTime,
    HD_Id,
    HD_Language,
    HD_Status,
    HD_UserId,
    HD_Flags,
    HD_BlessCount,
    HD_CurseCount,
    HD_BlockCount,
    HD_Count
  };

public:
  explicit AnnotationBrowser(SignalHub *hub, QWidget *parent = nullptr);

signals:
  void annotationTextUpdatedWithId(QString text, qint64 id);
  void annotationUserIdUpdatedWithId(qint64 userId, qint64 id);
  void annotationDeletedWithId(qint64 id);

  void userBlessedWithId(qint64 uid);
  void userCursedWithId(qint64 uid);
  void userBlockedWithId(qint64 uid);
  void userBlockedWithAlias(QString alias);
  void userAnalyticsRequested(qint64 uid);

  void annotationAnalyticsRequested();
  void annotationBlessedWithId(qint64 aid);
  void annotationCursedWithId(qint64 aid);
  void annotationBlockedWithId(qint64 aid);
  void annotationBlockedWithText(QString alias);

  // - Properties -
public:
  bool isEmpty() const;
  qint64 userId() const { return userId_; }

public slots:
  void setUserId(qint64 uid) { userId_ = uid; }
  void setAnnotations(const AnnotationList &l);
  void addAnnotation(const Annotation &annot);
  void addAnnotations(const AnnotationList &l);
  void removeAnnotationWithId(qint64 id);
  void removeAnnotations();
  void clear();

  void setAnnotationPos(qint64 pos);

  void updateAnnotationTextWithId(const QString &text, qint64 id);
  void updateAnnotationUserIdWithId(qint64 userId, qint64 id);

protected slots:
  void saveAnnotationText(const QString &text);
  void takeAnnotationOwnership();
  void invalidateFilters();

protected:
  QModelIndex currentIndex() const;
  qint64 currentId() const;
  qint64 currentUserId() const;
  QString currentText() const;
  QString currentUserAlias() const;

  int rowWithId(qint64 aid) const;

  // - Events -
public slots:
  void setVisible(bool visible) override;

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  void updateContextMenu();

//protected slots:
//  void dragEnterEvent(QDragEnterEvent *event) override;
//  void dragMoveEvent(QDragMoveEvent *event) override;
//  void dragLeaveEvent(QDragLeaveEvent *event) override;
//  void dropEvent(QDropEvent *event) override;

//signals:
//  void dragEnterEventReceived(QDragEnterEvent *event);
//  void dragMoveEventReceived(QDragMoveEvent *event);
//  void dragLeaveEventReceived(QDragLeaveEvent *event);
//  void dropEventReceived(QDropEvent *event);

  // - Implementations -
protected slots:
  void editAnnotation();
  void deleteAnnotation();
  void copyAnnotation();
  void blessAnnotation();
  void curseAnnotation();
  void blockAnnotation();

  void viewUser();
  void blockUser();
  void analyzeUser();

  void setMe(bool t);
  void setNow(bool t);
  void setSubtitle(bool t);

protected:
  static void setHeaderData(QAbstractItemModel *model);

public:  // TO BE MOVED TO A COMMON PLACE
  static QString languageToString(int lang);
  static QStringList annotationFlagsToStringList(int flags);
  static QString annotationStatusToString(int flags);

private:
  void createModel();
  void createLayout();
  void createContextMenu();

  AnnotationEditor *editor() const;

private:
  SignalHub *hub_;
  qint64 userId_;
  qint64 annotationPos_;

  AnnotationEditor *editor_;
  QModelIndex editedIndex_;

  // - Modes and UI components -
  QStandardItemModel *sourceModel_;
  QSortFilterProxyModel *proxyModel_;
  QSortFilterProxyModel *filterMeModel_, *filterNowModel_, *filterSubtitleModel_;
  AcFilteredTableView *tableView_;

  // - Menus and actions -
  QMenu *contextMenu_;
  QToolButton *meButton_, *nowButton_, *subtitleButton_;

  QAction *editAnnotAct_,
          *copyAnnotAct_,
          *blockAnnotAct_,
          *deleteAnnotAct_,
          *blessAnnotAct_,
          *curseAnnotAct_,
          *analyticsAct_;

  QAction //*viewUserAct_,
          *blockUserAct_,
          *analyzeUserAct_;
};

#endif // ANNOTATIONBROWSER_H
