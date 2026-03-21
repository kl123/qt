#include "forgetpwd.h"
#include "ui_forgetpwd.h"

ForgetPwd::ForgetPwd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ForgetPwd)
{
    ui->setupUi(this);
}

ForgetPwd::~ForgetPwd()
{
    delete ui;
}

void ForgetPwd::on_btnOk_clicked()
{
    QString username = ui->userEdit->text().trimmed();
    QString newPwd = ui->pwdEdit->text().trimmed();
    
    if (username.isEmpty() || newPwd.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名和新密码均不能为空！");
        return;
    }
    
    QString errMsg;
    if (UserAuth::resetPassword(username, newPwd, &errMsg)) {
        QMessageBox::information(this, "修改成功", "密码已重置，请使用新密码登录！");
        this->accept();
    } else {
        QMessageBox::critical(this, "修改失败", errMsg);
    }
}

void ForgetPwd::on_btnCancel_clicked()
{
    this->reject();
}
