// qxwebpage.cc
// 4/9/2012

#include "qtx/qxwebpage.h"
#include <QtGui>
#include <QtWebKit/QWebFrame>

#define DEBUG "qtext::webpage"
#include "qtx/qxdebug.h"

#ifdef __GNUC__
# pragma GCC diagnostic ignored "-Wparentheses" // suggest parentheses around assignment
#endif // __GNUC__

// - RC -

#ifdef Q_OS_LINUX
# ifndef JSFDIR
#  define JSFDIR      "/usr/share/annot"
# endif // JSFDIR
# define RC_PREFIX     JSFDIR "/"
#else
# define RC_PREFIX     QCoreApplication::applicationDirPath() + "/jsf/"
#endif // Q_OS_LINUX

//#define RC_HTML_ERROR   RC_PREFIX "error.html"
#define RC_JSF_ERROR    RC_PREFIX "error.xhtml"

namespace { namespace detail {
  //inline QByteArray rc_html_error_()
  //{
  //  QFile f(RC_HTML_ERROR);
  //  return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
  //}

  inline QByteArray rc_jsf_error()
  {
    static QByteArray ret;
    if (ret.isEmpty()) {
      QFile f(RC_JSF_ERROR);
      if (f.open(QIODevice::ReadOnly))
        ret = f.readAll();
    }
    return ret;
  }

  inline QByteArray rc_jsf_error(int error, const QString &reason, const QUrl &url)
  {
    return QString(rc_jsf_error())
      .replace("#{error}", QString::number(error))
      .replace("#{reason}", reason)
      .replace("#{url}", url.toString())
      .toUtf8();
  }
} } // anonymous detail

QTX_BEGIN_NAMESPACE

// - Construction -

QxWebPage::QxWebPage(QWidget *parent)
  : Base(parent)
{ connect(this, SIGNAL(linkHovered(QString,QString,QString)), SLOT(setHoveredLink(QString))); }

// - Events -

bool
QxWebPage::event(QEvent *event)
{
  Q_ASSERT(event);
  if (event->type() == QEvent::MouseButtonRelease) {
    auto e = static_cast<QMouseEvent *>(event);
    if (e->button() == Qt::LeftButton && e->modifiers() == Qt::ControlModifier)  {
      if (!hoveredLink_.isEmpty())
        emit openLinkRequested(hoveredLink_);
      e->accept();
      return true;
    }
  }
  return Base::event(event);
}


// - Extensions -

bool
QxWebPage::supportsExtension(Extension extension) const
{
  switch (extension) {
  case ErrorPageExtension: return true;
  default: return Base::supportsExtension(extension);
  }
}

bool
QxWebPage::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
  switch (extension) {
  case ErrorPageExtension:
    return errorPageExtension(static_cast<const ErrorPageExtensionOption *>(option),
                              static_cast<ErrorPageExtensionReturn *>(output))
        || Base::extension(extension, option, output);
  default:
    return Base::extension(extension, option, output);
  }
}

bool
QxWebPage::errorPageExtension(const ErrorPageExtensionOption *option, ErrorPageExtensionReturn *output)
{
  if (!option || !output)
    return false;
  DOUT("enter: error =" << option->error << ", message =" << option->errorString);
  output->baseUrl = option->url;
  output->content = detail::rc_jsf_error(option->error, option->errorString, option->url);
  //output->contentType = "text/html"; // automaticly detected
  output->encoding = "UTF-8";
  DOUT("exit");
  return true;
}

// - Scroll -

void
QxWebPage::scrollTop()
{ mainFrame()->setScrollBarValue(Qt::Vertical, 0); }

void
QxWebPage::scrollBottom()
{ mainFrame()->setScrollBarValue(Qt::Vertical, mainFrame()->scrollBarMaximum(Qt::Vertical)); }

void
QxWebPage::scrollLeft()
{ mainFrame()->setScrollBarValue(Qt::Horizontal, 0); }

void
QxWebPage::scrollRight()
{ mainFrame()->setScrollBarValue(Qt::Horizontal, mainFrame()->scrollBarMaximum(Qt::Horizontal)); }

// - User Agent -

// See: WebKit/qt/Api/qwebpage.cpp
// Example: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/534.34 (KHTML, like Gecko) MYAPP/MYVERSION Safari/534.34
QString
QxWebPage::userAgentForUrl(const QUrl &url) const
{
  static QString ret;
  if (ret.isEmpty())
    ret = Base::userAgentForUrl(url)
        .replace(QRegExp(" \\S+ Safari/"), " Safari/");
  return ret;
}

QTX_END_NAMESPACE

// EOF

/*
QObject*
QxWebPage::createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
  DOUT("classId =" << classid << ", url =" << url.toString() << ", paramNames =" << paramNames << ", paramValues =" << paramValues);
  return Base::createPlugin(classid, url, paramNames, paramValues);
}
*/
