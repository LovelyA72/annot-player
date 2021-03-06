#ifndef MRLRESOLVERSETTINGS_H
#define MRLRESOLVERSETTINGS_H

// mrlresolversettings.h
// 2/19/2011

#include <QtCore/QObject>
#include <QtCore/QString>

class MrlResolverSettings : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(MrlResolverSettings)
  typedef MrlResolverSettings Self;
  typedef QObject Base;

public:
  struct Account {
    QString username, password;

    explicit Account(const QString &name = QString(), const QString &pass = QString())
      : username(name), password(pass) { }
    bool isBad() const { return username.isEmpty() || password.isEmpty(); }
    void clear() { username.clear(); password.clear(); }
  };

  static Self *globalSettings() { static Self g; return &g; }

protected:
  explicit MrlResolverSettings(QObject *parent = nullptr) : Base(parent) { }

  // - Account -
public slots:
  void setNicovideoAccount(const QString &username = QString(),
                           const QString &password = QString())
  {
    nicovideoAccount_.username = username;
    nicovideoAccount_.password = password;
  }
  void setBilibiliAccount(const QString &username = QString(),
                          const QString &password = QString())
  {
    bilibiliAccount_.username = username;
    bilibiliAccount_.password = password;
  }

public:
  Account nicovideoAccount() const
  {
    return nicovideoAccount_.isBad() ? Account(CONFIG_NICOVIDEO_USERNAME, CONFIG_NICOVIDEO_PASSWORD)
                                     : nicovideoAccount_;
  }

  Account bilibiliAccount() const { return bilibiliAccount_; }

  bool hasNicovideoAccount() const { return true; }
  bool hasBilibiliAccount() const { return !bilibiliAccount_.isBad(); }

private:
  Account nicovideoAccount_;
  Account bilibiliAccount_;
};

#endif // MRLRESOLVERSETTINGS_H
