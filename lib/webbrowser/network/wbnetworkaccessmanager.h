#ifndef WBNETWORKACCESSMANAGER_H
#define WBNETWORKACCESSMANAGER_H

// network/wbnetwork.h
// 1/27/2012

#include <QtNetwork/QNetworkAccessManager>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

class WbNetworkAccessManager : public QNetworkAccessManager
{
  Q_OBJECT
  Q_DISABLE_COPY(WbNetworkAccessManager)
  typedef WbNetworkAccessManager Self;
  typedef QNetworkAccessManager Base;

  QList<QUrl> blockedUrls_;

public:
  explicit WbNetworkAccessManager(QObject *parent = nullptr)
    : Base(parent) { }

  static QStringList supportedSites();

public slots:
  void setBlockedUrls(const QList<QUrl> &urls) { blockedUrls_ = urls; }

protected:
  QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = nullptr) override;
  bool isBlockedUrl(const QUrl &url) const;
  static QUrl encodeNicoUrl(const QUrl &url);
  static QUrl encodeNicoWatchUrl(const QUrl &url);
  static QUrl encodeEroUrl(const QUrl &url);
  static QUrl encodeGetchuUrl(const QUrl &url);
  static QUrl encodeDmmUrl(const QUrl &url);

  static QUrl encodeAliceSoftUrl(const QUrl &url);
  static QUrl encodeAb2Url(const QUrl &url);
  static QUrl encodeAb3Url(const QUrl &url);
  static QUrl encodeAb2TryUrl(const QUrl &url);
  static QUrl encodeAkatsukiWorksUrl(const QUrl &url);
  static QUrl encodeWheelSoftUrl(const QUrl &url);
  static QUrl encodeSyangrilaUrl(const QUrl &url);
  static QUrl encodeCo2005Url(const QUrl &url);
  static QUrl encodeEffordomSoftUrl(const QUrl &url);
  static QUrl encodeAppliqueSoftUrl(const QUrl &url);
  static QUrl encodeShallotSoftUrl(const QUrl &url);
  static QUrl encodeSpermaniaxUrl(const QUrl &url);
  static QUrl encodeWitchFlameUrl(const QUrl &url);
  static QUrl encodeSpielSoftUrl(const QUrl &url);
  static QUrl encodeSpriteUrl(const QUrl &url);
};

#endif // WBNETWORKACCESSMANAGER_H
