// livedialog.cc
// 10/7/2011

#include "livedialog.h"
#include "tr.h"
#include "src/common/acui.h"
#include <QtGui>

#define SLOTLINE_MAXWIDTH 150

#define WINDOW_FLAGS ( \
  Qt::Dialog | \
  Qt::CustomizeWindowHint | \
  Qt::WindowTitleHint | \
  Qt::WindowCloseButtonHint | \
  Qt::WindowStaysOnTopHint )

// - Constructions -

LiveDialog::LiveDialog(QWidget *parent)
  : Base(parent, WINDOW_FLAGS), timeSlotIndex_(0)
{
  setWindowTitle(TR(T_TITLE_LIVE));

  createLayout();

  // Focus
  comboBox_->setFocus();
}

void
LiveDialog::createLayout()
{
  AcUi *ui = AcUi::globalInstance();

  comboBox_ = ui->makeComboBox();
  comboBox_->setMaximumWidth(SLOTLINE_MAXWIDTH);

  QToolButton *okButton = ui->makeToolButton(
        AcUi::PushHint | AcUi::HighlightHint, TR(T_OK), this, SLOT(ok()));
  QToolButton *cancelButton = ui->makeToolButton(
        AcUi::PushHint, TR(T_CANCEL), this, SLOT(fadeOut()));

  QLabel *comboBoxLabel = ui->makeLabel(
        AcUi::BuddyHint, TR(T_LABEL_TIMESLOT), TR(T_TOOLTIP_TIMESLOT), comboBox_);

  // Layouts
  QVBoxLayout *rows = new QVBoxLayout; {
    QHBoxLayout *row1 = new QHBoxLayout,
                *row2 = new QHBoxLayout;
    rows->addLayout(row1);
    rows->addLayout(row2);

    row1->addWidget(comboBoxLabel);
    row1->addWidget(comboBox_);
    row2->addWidget(okButton);
    row2->addWidget(cancelButton);

    setContentsMargins(0, 0, 0, 0);
  } setLayout(rows);

  /*
  QGridLayout *layout = new QGridLayout; {
    layout->addWidget(userNameLabel, 0, 0);
    layout->addWidget(userNameEdit_, 0, 1);
    layout->addWidget(passwordLabel, 1, 0);
    layout->addWidget(passwordEdit_, 1, 1);
    layout->addWidget(okButton, 2, 0);
    layout->addWidget(cancelButton, 2, 1);
  }
  setLayout(layout);
  */

  // Connections
  connect(cancelButton, SIGNAL(clicked()), SLOT(cancel()));
  connect(okButton, SIGNAL(clicked()), SLOT(ok()));

  connect(comboBox_, SIGNAL(currentIndexChanged(int)), SLOT(setTimeSlotIndex(int)));
}

void
LiveDialog::setVisible(bool visible)
{
  if (visible)
    updateComboBox();
  Base::setVisible(visible);
}

void
LiveDialog::updateComboBox()
{
  comboBox_->clear();

  for (int i = 0; i < timeSlots_.size(); i++) {
    qint64 secs = timeSlots_[i];
    qint64 msecs = secs * 1000;
    QTime t = QDateTime::fromMSecsSinceEpoch(msecs).time();
    QString ts = t.toString();

    int count = 0;
    if (i < activeCounts_.size())
      count = activeCounts_[i];

    QString text;
    if (count)
      text = QString("%1 (%2)").arg(ts).arg(count);
    else
      text = ts;
    comboBox_->addItem(text);
  }
}

// - Properties -

int
LiveDialog::timeSlotIndex() const
{ return timeSlotIndex_; }

void
LiveDialog::setTimeSlotIndex(int index)
{ timeSlotIndex_ = index; }

const QList<qint64>&
LiveDialog::timeSlots() const
{ return timeSlots_; }

void
LiveDialog::setTimeSlots(const QList<qint64> &timeSlots)
{ timeSlots_ = timeSlots; }

const QList<int>&
LiveDialog::activeCounts() const
{ return activeCounts_; }

void
LiveDialog::setActiveCounts(const QList<int> &counts)
{ activeCounts_ = counts; }

// - Slots -

void
LiveDialog::ok()
{
  if (timeSlotIndex_ >= 0 && timeSlotIndex_ < timeSlots_.size()) {
    qint64 slot = timeSlots_[timeSlotIndex_];
    emit timeSlotSelected(slot);
  }
}

// EOF
