#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
// 布局/控件头文件
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
#include <QWidget>
#include <QButtonGroup>
// 业务依赖
#include <QMessageBox>
#include "userauth.h"
#include <QList>


class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login() override; // 虚析构，确保子类析构正常

private slots:
    // 登录标签页槽
    void on_btnLogin_clicked();
    void on_btnLoginCancel_clicked();
    void on_btnForgetPwd_clicked();
    // 注册标签页槽
    void on_btnRegister_clicked();
    void on_btnRegReset_clicked();
    void on_rbtnDispatcher_toggled(bool checked);
    void on_btnSearchDept_clicked();

private:
    // #################### 所有UI控件指针（纯代码创建，无.ui） ####################
    QTabWidget *m_tabWidget;
    // 登录标签页
    QWidget *m_widLogin;
    QLabel *m_labLoginUser;
    QLineEdit *m_leLoginUser;
    QLabel *m_labLoginPwd;
    QLineEdit *m_leLoginPwd;
    QWidget *m_widLoginBtn;
    QPushButton *m_btnLogin;
    QPushButton *m_btnLoginCancel;
    QPushButton *m_btnForgetPwd; // 忘记密码按钮
    // 注册标签页
    QWidget *m_widRegister;
    // 通用注册项
    QLabel *m_labRegUser;
    QLineEdit *m_leRegUser;
    QLabel *m_labRegPwd;
    QLineEdit *m_leRegPwd;
    QLabel *m_labRegPwdConfirm;
    QLineEdit *m_leRegPwdConfirm;
    QLabel *m_labRegPhone;
    QLineEdit *m_leRegPhone;
    // 身份选择
    QGroupBox *m_gboxRole;
    QRadioButton *m_rbtnDispatcher;
    QRadioButton *m_rbtnHandler;
    QButtonGroup *m_btnGroupRole;
    // 指挥调度员专属
    QWidget *m_widDispatcher;
    QLabel *m_labDeptCreate;
    QLineEdit *m_leDeptCreate;
    // 现场处置员专属
    QWidget *m_widHandler;
    QLabel *m_labDeptSearch;
    QLineEdit *m_leDeptSearch;
    QPushButton *m_btnSearchDept;
    QLabel *m_labDispatcherSel;
    QComboBox *m_cboDispatchers;
    // 注册按钮组
    QWidget *m_widRegBtn;
    QPushButton *m_btnRegister;
    QPushButton *m_btnRegReset;

    // #################### 业务成员变量 ####################
    qint64 m_selectedUnitId = 0;     // 选中的单位ID
    qint64 m_selectedDispatcherId = 0;// 选中的调度员ID

    // #################### 私有方法 ####################
    void createUI();                // 纯代码创建所有UI
    void buildLoginTab();           // 构建登录标签页
    void buildRegisterTab();        // 构建注册标签页
    bool checkRegForm();            // 注册表单验证
    void resetRegForm();            // 重置注册表单
};

#endif // LOGIN_H
