//
// Created by darren on 10/04/2020.
//

#ifndef QLAM_TIMEDACTIONDIALOGUE_H
#define QLAM_TIMEDACTIONDIALOGUE_H

#include <QtWidgets/QDialog>
#include <QtCore/QTimer>

namespace Ui {
    class TimedActionDialogue;
}

namespace Qlam {

    class TimedActionDialogue
    : public QDialog {

        Q_OBJECT

        public:
            using Action = std::function<void()>;

            TimedActionDialogue(QString, Action, QWidget * = nullptr);
            TimedActionDialogue(QString, Action, int, QWidget * = nullptr);
            ~TimedActionDialogue() override;

            [[nodiscard]] inline QString message() const {
                return m_messageTemplate;
            }

            void setMessage(const QString &);

            [[nodiscard]] inline int timeout() const {
                return m_timeout;
            }

            bool setTimeout(int timeout);

            [[nodiscard]] inline int timeRemaining() const {
                return m_remaining;
            }

        public Q_SLOTS:
            bool start(std::optional<int> timeout = {});
            bool pause();
            bool resume();
            virtual void cancel();

        protected:
            virtual void triggerAction();
            void refreshMessage();

        private:
            void slotTimerTick();

            std::unique_ptr<Ui::TimedActionDialogue> m_ui;
            int m_timeout;
            int m_remaining;
            QString m_messageTemplate;
            Action m_action;
            QTimer m_timer;
    };

}

#endif //QLAM_TIMEDACTIONDIALOGUE_H
