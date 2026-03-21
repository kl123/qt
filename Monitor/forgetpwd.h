#ifndef FORGETPWD_H
#define FORGETPWD_H

#include <QDialog>
#include <QMessageBox>
#include "userauth.h"

namespace Ui {
class ForgetPwd;
}

class ForgetPwd : public QDialog
{
    Q_OBJECT

public:
    explicit ForgetPwd(QWidget *parent = nullptr);
    ~ForgetPwd();

private slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

private:
    Ui::ForgetPwd *ui;
};

#endif // FORGETPWD_H
