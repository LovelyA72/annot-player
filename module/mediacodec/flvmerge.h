#ifndef FLVMERGE_H
#define FLVMERGE_H

// flvmerge.h
// 3/14/2012

#include "mediacodec_config.h"
#include "module/stream/inputstream.h"
#include "module/stream/outputstream.h"
#include "module/stream/streampipe.h"
#include <QtCore/QObject>

// - Merger -

class FlvMerge : public QObject, public StreamListPipe
{
  Q_OBJECT
  typedef FlvMerge Self;
  typedef QObject Base;

  enum State { Error = -1, Stopped = 0, Running, Finished };
  State state_;

  InputStreamList ins_;
  OutputStream *out_;
  qint64 audioTimestamp_, videoTimestamp_,
         lastAudioTimestamp_, lastVideoTimestamp_,
         lastAudioTimestep_, lastVideoTimestep_;
  qint64 duration_;

  bool audioTagStripped_,
       videoTagStripped_,
       scriptTagStripped_;

public:
  FlvMerge(const InputStreamList &ins, OutputStream *out, QObject *parent = 0)
    : Base(parent), state_(Stopped), ins_(ins), out_(out),
      audioTimestamp_(0), videoTimestamp_(0), duration_(0) { }

  explicit FlvMerge(const InputStreamList &ins, QObject *parent = 0)
    : Base(parent), state_(Stopped), ins_(ins), out_(0),
      audioTimestamp_(0), videoTimestamp_(0), duration_(0) { }

  explicit FlvMerge(QObject *parent = 0)
    : Base(parent), out_(0),
      audioTimestamp_(0), videoTimestamp_(0), duration_(0) { }

public:
  bool isStopped() const { return state_ == Stopped; }
  bool isRunning() const { return state_ == Running; }
  bool isFinished() const { return state_ == Finished; }

  void setInputStreams(const InputStreamList &ins) { ins_ = ins; } ///< \override
  void setOutputStream(OutputStream *out) { out_ = out; } ///< \override
  void setDuration(qint64 msec) { duration_ = msec; }

  qint64 duration() const { return duration_; }

  qint64 timestamp() const { return qMax(audioTimestamp_, videoTimestamp_); }

signals:
  void stopped();
  //void timestampChanged(qint64);
  void error(QString message);

public slots:
  virtual void run(); ///< \override

  virtual void stop() ///< \override
  {
    state_ = Stopped;
    emit stopped();
  }

  void finish();

public:
  bool parse();
  bool merge();

protected:
  InputStreamList inputStreams() const { return ins_; } ///< \override
  OutputStream *outputStream()  const { return out_; } ///< \override

  bool append(InputStream *in, bool writeHeader);
  bool readTag(InputStream *in, bool writeHeader);
  int updateScriptTag(QByteArray &data, int offset) const;
private:
  bool updateScriptTagDoubleValue(quint8 *data, const QString &var) const;
  bool updateScriptTagUInt8Value(quint8 *data, const QString &var) const;
};

#endif // FLVMERGE_H