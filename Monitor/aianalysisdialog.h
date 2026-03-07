#ifndef AIANALYSISDIALOG_H
#define AIANALYSISDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>

class AiAnalysisDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AiAnalysisDialog(QWidget *parent = nullptr);
    void appendLog(const QString &msg);

private:
    QTextEdit *m_logEdit;
    QPushButton *m_closeButton;
};

#endif // AIANALYSISDIALOG_H
