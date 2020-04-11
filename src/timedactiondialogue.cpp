//
// Created by darren on 10/04/2020.
//

#include <cmath>
#include <QPushButton>
#include "timedactiondialogue.h"
#include "ui/ui_timedactiondialogue.h"

using namespace Qlam;

static constexpr const int DefaultTimeout = 5000;
static constexpr const int TimerResolution = 100;

TimedActionDialogue::TimedActionDialogue(QString message, Qlam::TimedActionDialogue::Action action, QWidget * parent)
:   TimedActionDialogue(std::move(message), std::move(action), DefaultTimeout, parent)
{
}

TimedActionDialogue::TimedActionDialogue(QString message, TimedActionDialogue::Action action, int timeout, QWidget * parent)
:   QDialog(parent),
    m_ui(std::make_unique<Ui::TimedActionDialogue>()),
    m_messageTemplate(std::move(message)),
    m_action(std::move(action)),
    m_timer(),
    m_timeout(timeout),
    m_remaining(timeout),
    m_cancelLabel(tr("Cancel")),
    m_nowLabel(tr("Do It Now"))
{
    m_ui->setupUi(this);
    m_timer.setInterval(TimerResolution);

    connect(m_ui->controls, &QDialogButtonBox::rejected, this, &TimedActionDialogue::cancel);
    connect(m_ui->controls, &QDialogButtonBox::accepted, this, &TimedActionDialogue::performActionNow);
    connect(&m_timer, &QTimer::timeout, this, &TimedActionDialogue::slotTimerTick);

    refreshMessage();
    refreshButtonLabels();
}

TimedActionDialogue::~TimedActionDialogue() = default;

void TimedActionDialogue::setMessage(const QString & message)
{
    m_messageTemplate = message;
    refreshMessage();
}

void TimedActionDialogue::refreshMessage() {
    m_ui->message->setText(m_messageTemplate.arg(static_cast<int>(std::ceil(static_cast<double>(timeRemaining()) / 1000.0))));
}

bool TimedActionDialogue::setTimeout(int timeout)
{
    if (0 > timeout) {
        return false;
    }

    if (m_timer.isActive()) {
        return false;
    }

    m_timeout = timeout;
    m_remaining = timeout;
    return true;
}

bool TimedActionDialogue::start(std::optional<int> timeout)
{
    if (m_timer.isActive()) {
        return false;
    }

    if (timeout && 0 <= timeout.value()) {
        setTimeout(timeout.value());
    }

    m_timer.start();
    return true;
}

void TimedActionDialogue::slotTimerTick()
{
    m_remaining -= TimerResolution;

    if (0 > m_remaining) {
        performActionNow();
        return;
    }

    refreshMessage();
}

void TimedActionDialogue::performActionNow()
{
    m_timer.stop();
    triggerAction();
    close();
}

bool TimedActionDialogue::pause()
{
    if (!m_timer.isActive()) {
        return false;
    }

    m_timer.stop();
    return true;
}

bool TimedActionDialogue::resume()
{
    if(m_timer.isActive()) {
        return false;
    }

    if (m_remaining == m_timeout) {
        return false;
    }

    m_timer.start();
    return true;
}

void TimedActionDialogue::triggerAction()
{
    this->m_action();
}

void TimedActionDialogue::cancel()
{
    reset();
    close();
}

void TimedActionDialogue::reset()
{
    if(m_timer.isActive()) {
        m_timer.stop();
    }

    m_remaining = m_timeout;
}

void TimedActionDialogue::enableCancel(bool enable)
{
    QDialogButtonBox::StandardButtons buttons = m_ui->controls->standardButtons();

    if (enable) {
        buttons |= QDialogButtonBox::Cancel;
    } else {
        buttons &= ~QDialogButtonBox::Cancel;
    }

    m_ui->controls->setStandardButtons(buttons);
    refreshCancelLabel();
}

void TimedActionDialogue::enableNow(bool enable)
{
    QDialogButtonBox::StandardButtons buttons = m_ui->controls->standardButtons();

    if (enable) {
        buttons |= QDialogButtonBox::Ok;
    } else {
        buttons &= ~QDialogButtonBox::Ok;
    }

    m_ui->controls->setStandardButtons(buttons);
    refreshNowLabel();
}

void TimedActionDialogue::setCancelLabel(const QString & label)
{
    m_cancelLabel = label;
    refreshCancelLabel();
}

void TimedActionDialogue::setNowLabel(const QString & label)
{
    m_nowLabel = label;
    refreshNowLabel();
}

void TimedActionDialogue::refreshButtonLabels()
{
    refreshCancelLabel();
    refreshNowLabel();
}

void TimedActionDialogue::refreshCancelLabel()
{
    QPushButton * button = m_ui->controls->button(QDialogButtonBox::Ok);

    if (!button) {
        return;
    }

    return button->setText(m_cancelLabel);
}

void TimedActionDialogue::refreshNowLabel()
{
    QPushButton * button = m_ui->controls->button(QDialogButtonBox::Ok);

    if (!button) {
        return;
    }

    return button->setText(m_nowLabel);
}
