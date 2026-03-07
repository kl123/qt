#include "aianalysisdialog.h"
#include <QVBoxLayout>

AiAnalysisDialog::AiAnalysisDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("AI 思考与分析过程");
    resize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    
    m_logEdit = new QTextEdit(this);
    m_logEdit->setReadOnly(true);
    m_logEdit->setStyleSheet("QTextEdit { background-color: #f0f0f0; font-family: Consolas; }");
    layout->addWidget(m_logEdit);

    m_closeButton = new QPushButton("关闭", this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(m_closeButton);
}

void AiAnalysisDialog::appendLog(const QString &msg)
{
    m_logEdit->append(msg);
    // 自动滚动到底部
    m_logEdit->moveCursor(QTextCursor::End);
}
