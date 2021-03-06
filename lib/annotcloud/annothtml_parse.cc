// annothtml_parse.cc
// 8/19/2011
//
// Qt CSS restriction:
// doc.trolltech.com/4.8/richtext-html-subset.html

#include "lib/annotcloud/annothtml.h"
#include "lib/annotcloud/annottag.h"
#include "htmlutil/htmltags.h"
#include <boost/tuple/tuple.hpp>
#include <stack>

//#define DEBUG "annothtmlparse"
#include "qtx/qxdebug.h"

#define MAX_REPEAT_COUNT        10

// - Lexical parsing -

// Regexp is not used to improve performance.
std::pair<QString, QString>
AnnotCloud::
AnnotationHtmlParser::parseLeadingTag(const QString &text)
{
  typedef std::pair<QString, QString> RETURN;

  if (text.isEmpty() || text[0] != CORE_CMDCH)
    return RETURN();

  // TODO: use while
  //int index = 0;
  //foreach (const QChar &c, text)
  //  if (isSeparator(c))
  //    break;
  //  else
  //    index++;
  //if (index >= text.size())
  int index = text.indexOf(' ');
  return index < 0 ? RETURN(text, QString()) :
                     RETURN(text.left(index), text.mid(index + 1));
}

std::pair<QString, QString>
AnnotCloud::
AnnotationHtmlParser::parseNextToken(const QString &text)
{
#define SELF(_text) parseNextToken(_text)
  typedef std::pair<QString, QString> RETURN;

  switch (text.size()) {
  case 0: return RETURN();
  case 1: return RETURN(text, QString());
  }

  ushort ch0 = text[0].unicode();
  switch (ch0) {
  case ' ':
  case '\n':
    return text.size() == 1 ? RETURN() : SELF(text.mid(1));
  case '{': case '}':
  case '[': case ']':
    return RETURN(text[0], text.mid(1));
  }

  int mid = 0;
  for (int i = 1; !mid && i < text.size(); i++) {
    switch (text[i].unicode()) {
    case ' ':
    case '\n':
    case '<': case '>':
      if (ch0 == CORE_CMDCH)
        mid = i;
      break;

    case CORE_CMDCH:
    case '{': case '}':
    case '[': case ']':
      if (ch0 == CORE_CMDCH && i == 1)
        mid = i + 1;
      else
        mid = i;
      break;
    }
  }
  return mid ? RETURN(text.left(mid), text.mid(mid).trimmed()) :
               RETURN(text, QString());
#undef SELF
}

// - Syntax parsing -

std::pair<QString, QStringList>
AnnotCloud::
AnnotationHtmlParser::toHtml(const QString &text) const
{
  DOUT("enter: text=" << text);

  typedef std::pair<QString, QStringList> RETURN;
  if (text.isEmpty()) {
    DOUT("exit");
    return RETURN();
  }

  QString parsed;
  QString unparsed = text;

  QStringList tags;

  enum StringType {   // actually not used
    ST_None = '\0',
    ST_Tag = CORE_CMDCH,
    ST_LeftCurlyBracket = '{',
    ST_RightCurlyBracket = '}',
    ST_LeftSquareBracket = '[',
    ST_RightSquareBracket = ']' };

  std::stack<std::pair<QString, ushort> > stack; // pair<QString, StringType>

  // FIXME: add support for verbatim
  //if (text.startsWith(CORE_CMD_VERBATIM)) {
  //  parsed = text.mid(QString(CORE_CMD_VERBATIM).size());
  //  tags.prepend(CORE_CMD_VERBATIM);
  //  DOUT("exit: parsed (verbatim) =" << parsed);
  //  return RETURN(parsed.trimmed(), tags);
  //}
  if (text.startsWith(CORE_CMD_V " ")) { // \verbatim alias
    parsed = text.mid(QString(CORE_CMD_V " ").size());
    tags.prepend(CORE_CMD_V);
    DOUT("exit: parsed (verbatim) =" << parsed);
    return RETURN(parsed.trimmed(), tags);
  }

  do {
    QString current;
    boost::tie(current, unparsed) = parseNextToken(unparsed);
    if (current.isEmpty()) {
      // i.e. EOL
      // REDUCE until stack empty

      QStringList params, attrs;
      while (!stack.empty()) {
        auto front = stack.top();
        stack.pop();

        switch (front.second) {

        case '}':
          params.prepend(front.first);
          break;
        case ']':
          attrs.prepend(front.first);
          break;

        case CORE_CMDCH: {
            if (!parsed.isEmpty())
              params.append(parsed);

            QString t = translate(front.first, params, attrs);
            if (!t.isEmpty()) {
              parsed = t;
            } else {
              tags.prepend(front.first);
              parsed = params.join(QString());
              if (!attrs.isEmpty())
                DOUT("attributes of unknown tag ignored:" << front.first);
            }

            params.clear();
            if (!attrs.isEmpty())
              attrs.clear();
          } break;

        case '{':
        case '[':
          DOUT("warning: unparsed parentheses on the stack");
        case '\0':
          // Allow post-fixed attributes
          //if (!attrs.isEmpty())
          //  attrs.clear();
          if (!params.isEmpty()) {
            parsed = params.join(QString()) + parsed;
            params.clear();
          }
          parsed = front.first + parsed;
          break;

        default: Q_ASSERT(0);
        }
      }
      if (!params.isEmpty())
        parsed = params.join(QString()) + parsed;
      break;
    }

    ushort current_ch = current[0].unicode();
    switch (current_ch) {
    case '}':
      if (stack.empty()) {
        DOUT("exit: '{}' mismatched");
        return RETURN();
      } else {
        // REDUCE until '{'
        QString reduced;
        QStringList params, attrs;
        if (stack.empty())
          DOUT("parse: '^]' mismatched");
        while (!stack.empty()) {
          auto front = stack.top();
          stack.pop();

          switch (front.second) {
          case '{':
            break;
          case '[':
            DOUT("parsed: '[{' mismatched");
            break;

          case '}':
            params.prepend(front.first);
            break;
          case ']':
            attrs.prepend(front.first);
            break;

          case CORE_CMDCH: {
            if (!reduced.isEmpty())
              params.append(reduced);
            QString t = translate(front.first, params, attrs);
            if (!t.isEmpty()) {
              reduced = t;
            } else {
              tags.prepend(front.first);
              reduced = params.join(QString());
              if (!attrs.isEmpty())
                DOUT("attributes of unknown tag ignored:" << front.first);
            }
            params.clear();
            if (!attrs.isEmpty())
              attrs.clear();
          } break;

          case '\0':
            // Allow post-fixed attributes
            //if (!attrs.isEmpty())
            //  attrs.clear();
            if (!params.isEmpty()) {
              reduced = params.join(QString()) + reduced;
              params.clear();
            }
            reduced = front.first + reduced;
            break;

          default:
            Q_ASSERT(0);
          }

          if (front.second == '{')
            break;
        }

        if (!params.isEmpty())
          reduced = params.join(QString()) + reduced;
        if (!reduced.isEmpty())
          stack.push(std::make_pair(reduced, '}'));

        DOUT("reduced {}:" << reduced);
      } break;

    case ']':
      if (stack.empty()) {
        DOUT("exit: '[]' mismatched");
        return RETURN();
      } else {
        // REDUCE until '['
        QString reduced;
        if (stack.empty())
          DOUT("parse: '^]' mismatched");
        while (!stack.empty()) {
          auto front = stack.top();
          stack.pop();

          switch (front.second) {
          case '[':
            break;
          default:
            reduced = front.first + reduced;
          }

          if (front.second == '[')
            break;
        }

        if (!reduced.isEmpty())
          stack.push(std::make_pair(reduced, ']'));

        DOUT("reduced []:" << reduced);
      } break;


    case '{':
    case '[':
      stack.push(std::make_pair(current, current_ch));
      break;

    case CORE_CMDCH:
      if (current.size() == 2) {
        ushort ch1 = current[1].unicode();
        switch (ch1) {
        case CORE_CMDCH:
        case '{': case '}':
        case '[': case ']':
          current = ch1;
          current_ch = '\0';
          break;
        }
      }

      stack.push(std::make_pair(current, current_ch));
      break;

    default:
      stack.push(std::make_pair(current, '\0'));
    }

  } while (!stack.empty());

  DOUT("exit: parsed =" << parsed);
  return RETURN(parsed, tags);
}

QString
AnnotCloud::
AnnotationHtmlParser::translate(const QString &tag,
                            const QStringList &params,
                            const QStringList &attrs) const
{
  DOUT("enter: tag =" << tag);
  #ifdef DEBUG
  {
    qDebug() << "- attrs:";
    foreach (const QString &attr, attrs)
      qDebug() << attr;
    qDebug() << "- params:";
    foreach (const QString &param, params)
      qDebug() << param;
  }
  #endif // DEBUG

  if (tag.isEmpty())
    return QString();

  switch (qHash(tag)) {
  case H_Verbatim:
    return params.isEmpty() ? QString() : params.join(QString());

  case H_N:
  case H_Br:
  case H_LineBreak:
    return params.isEmpty() ? html_br()
    : html_br() + params.join(QString());

#define CASE_COLOR(_hash, _csstag) \
  case _hash: \
    if (params.isEmpty()) \
      return QString(); \
    else { \
      if (attrs.isEmpty()) { \
        if (params.size() == 1) \
          return params.first(); \
        else { \
          QString ret; \
          int i = 0; \
          foreach (const QString &param, params) { \
            switch (i) { \
            case 0: ret = html_ss_open(_csstag ":" + param); break; \
            case 1: ret += param + html_ss_close(); break; \
            default: ret += param; \
            } \
            i++; \
          } \
          return ret; \
        } \
      } else { \
        QString ret; \
        QString color = attrs.first(); \
        int i = 0; \
        foreach (const QString &param, params) { \
          if (i == 0) \
            ret = html_ss(param, _csstag ":" + color); \
          else \
            ret += param; \
          i++; \
        } \
        return ret; \
      } \
    } \
    Q_ASSERT(0); \
    break;

    CASE_COLOR(H_Color, "color")
    CASE_COLOR(H_ForegroundColor, "color")
    CASE_COLOR(H_BackgroundColor, "background-color")
#undef CASE_COLOR

  case H_Size:
    if (params.isEmpty())
      return QString();
    else {
      if (attrs.isEmpty()) {
        if (params.size() == 1)
          return params.first();
        else {
          QString ret;
          int i = 0;
          foreach (QString param, params) {
            if (param.isEmpty())
              continue;
            if (param[param.size() -1].isDigit())
              param += "px";
            switch (i) {
            case 0: ret = html_ss_open("font-size:" + param); break;
            case 1: ret += param + html_ss_close(); break;
            default: ret += param;
            }
            i++;
          }
          return ret;
        }
      } else {
        QString ret;
        QString size = attrs.first();
        if (!size.isEmpty() && size[size.size() -1].isDigit())
          size += "px";
        int i = 0;
        foreach (const QString &param, params) {
          if (i == 0)
            ret = html_ss(param, "font-size:" + size);
          else
            ret += param;
          i++;
        }
        return ret;
      }
    }
    Q_ASSERT(0);
    break;

  case H_Repeat:
    if (params.isEmpty())
      return QString();
    else {
      if (attrs.isEmpty()) {
        if (params.size() == 1)
          return params.first();
        else {
          QString ret;
          int count = 0;
          bool ok;
          int i = 0;
          foreach (const QString &param, params) {
            switch (i) {
            case 0:
              count = param.toInt(&ok);
              if (!ok)
                return QString();
              break;
            case 1:
              if (count > 0) {
                if (count > MAX_REPEAT_COUNT)
                  count = MAX_REPEAT_COUNT;
                ret = param.repeated(count);
              }
            default:
              ret += param;
            }
            i++;
          }
          return ret;
        }

      } else {
        QString ret;
        QString s_count = attrs.first();
        int count = s_count.toInt();
        if (count > 0) {
          if (count > MAX_REPEAT_COUNT)
            count = MAX_REPEAT_COUNT;
          int i = 0;
          foreach (const QString &param, params) {
            if (i == 0)
              ret = param.repeated(count);
            else
              ret += param;
            i++;
          }
        }
        return ret;
      }
    }
    Q_ASSERT(0);
    break;

  case H_Style:
    if (params.isEmpty())
      return QString();
    else {
      if (attrs.isEmpty()) {
        if (params.size() == 1)
          return params.first();
        else {
          QString ret;
          int i = 0;
          foreach (const QString &param, params) {
            switch (i) {
            case 0: {
                QString style = param;
                style.replace('=', ':').replace(',', ';');
                ret = html_ss_open(style);
              } break;
            case 1:
              ret += param + html_ss_close();
              break;
            default:
              ret += param;
            }
            i++;
          }
          return ret;
        }

      } else {
        QString ret;
        QString style = attrs.join(";");
        style.replace('=', ':').replace(',', ';');
        int i = 0;
        foreach (const QString &param, params) {
          if (i == 0)
            ret = html_ss(param, style);
          else
            ret += param;
          i++;
        }
        return ret;
      }
    }
    Q_ASSERT(0);
    break;

  case H_Font:
    if (params.isEmpty())
      return QString();
    else {
      if (attrs.isEmpty()) {
        if (params.size() == 1)
          return params.first();
        else {
          QString ret;
          int i = 0;
          foreach (const QString &param, params) {
            switch (i) {
            case 0: {
                QString attr = param;
                attr.replace(':', '=').replace(';', ' ').replace(',', ' ');
                ret = html_font_open(attr);
              } break;
            case 1:
              ret += param + html_font_close();
              break;
            default:
              ret += param;
            }
            i++;
          }
          return ret;
        }

      } else {
        QString ret;
        QString attr = attrs.join(" ");
        attr.replace(':', '=').replace(';', ' ').replace(',', ' ');
        int i = 0;
        foreach (const QString &param, params) {
          if (i == 0)
            ret = html_font(param, attr);
          else
            ret += param;
          i++;
        }
        return ret;
      }
    }
    Q_ASSERT(0);
    break;

#define RETURN_HTML_TAG(_tag) \
  { \
    switch (params.size()) { \
    case 0: return html_##_tag(); \
    case 1: return html_##_tag(params.first()); \
    default: { \
        QString ret; \
        int i = 0; \
        foreach (const QString &param, params) { \
          if (i == 0) \
            ret = html_##_tag(param); \
          else \
            ret += param; \
          i++; \
        } \
        return ret; \
      } \
    } \
    Q_ASSERT(0); \
    break; \
  }

  case H_Div:
    RETURN_HTML_TAG(div)

  case H_Em:
  case H_Emph:
  case H_Italic:
  case H_TextIt:
    RETURN_HTML_TAG(em)

  case H_B:
  case H_Bf:
  case H_Bold:
  case H_Strong:
  case H_TextBf:
    RETURN_HTML_TAG(strong)

#undef RETURN_HTML_TAG

#define RETURN_HTML_SS(_HTML, _style) \
  { \
    switch (params.size()) { \
    case 0: return QString(); \
    case 1: return _HTML(+params.first()+, _style); \
    default: { \
        QString ret; \
        int i = 0; \
        foreach (const QString &param, params) { \
          if (i == 0) \
            ret = _HTML(+param+, _style); \
          else \
            ret += param; \
          i++; \
        } \
        return ret; \
      } \
    } \
    Q_ASSERT(0); \
    break; \
  }

#define RETURN_html_ss(_html, _style) \
  { \
    switch (params.size()) { \
    case 0: return QString(); \
    case 1: return _html(params.first(), _style); \
    default: { \
        QString ret; \
        int i = 0; \
        foreach (const QString &param, params) { \
          if (i == 0) \
            ret = _html(param, _style); \
          else \
            ret += param; \
          i++; \
        } \
        return ret; \
      } \
    } \
    Q_ASSERT(0); \
    break; \
  }

  case H_LineThrough:
  case H_Strike:
  case H_StrikeOut:
  case H_Sout:
    RETURN_HTML_SS(HTML_SS, text-decoration:line-through)

  case H_Underline:
  case H_Uline:
  case H_Url: // Temporary rendering solution
    RETURN_HTML_SS(HTML_SS, text-decoration:underline)

  case H_Overline:
    RETURN_HTML_SS(HTML_SS, text-decoration:overline)

  case H_Blink:
    RETURN_HTML_SS(HTML_SS, text-decoration:blink)

#define CASE_TRANSFORM(_trans) \
  case H_##_trans: \
    RETURN_HTML_SS(HTML_SS, text-transform:_trans)

  CASE_TRANSFORM(Uppercase)
  CASE_TRANSFORM(Lowercase)
#undef CASE_TRANSFORM

#define CASE_COLOR(_color) \
  case H_##_color: \
    RETURN_HTML_SS(HTML_SS, color:_color)

  CASE_COLOR(Black)
  CASE_COLOR(Blue)
  CASE_COLOR(Brown)
  CASE_COLOR(Cyan)
  CASE_COLOR(Green)
  CASE_COLOR(Gray)
  CASE_COLOR(Grey)
  CASE_COLOR(Magenta)
  CASE_COLOR(Orange)
  CASE_COLOR(Pink)
  CASE_COLOR(Purple)
  CASE_COLOR(Red)
  CASE_COLOR(White)
  CASE_COLOR(Yellow)
#undef CASE_COLOR

#define CASE_SIZE(_id, _size) \
  case H_##_id: \
    RETURN_html_ss(html_ss, "font-size:" + _size)

  CASE_SIZE(Tiny,       tinySize_)
  CASE_SIZE(Small,      smallSize_)
  CASE_SIZE(Normal,     normalSize_)
  CASE_SIZE(Big,        largeSize_)
  CASE_SIZE(Large,      largeSize_)
  CASE_SIZE(Huge,       hugeSize_)
#undef CASE_SIZE

#undef RETURN_HTML_SS
#undef RETURN_html_ss

    // Unknown tag
  default:
    return QString();
  }
}

// EOF
