#include "login.h"
// Qt核心库
#include <QCoreApplication>
#include <QSizePolicy>
#include <QButtonGroup>
#include <QVariant>
#include <QColor>
#include <QMessageBox>
#include <QStyle> // 图片缩放用

// 全局布局常量（登录页更紧凑，注册页保持优化后）
const int LAYOUT_MARGIN = 15;    // 外边缘间距
const int LAYOUT_SPACING = 10;   // 注册页控件间距
const int SMALL_SPACING = 6;     // 小间距（同行/按钮）
const int LOGIN_SPACING = 8;     // 登录页专属更小间距，更紧凑

Login::Login(QWidget *parent) : QDialog(parent)
{
    // 窗口基础属性
    this->setWindowTitle("用户登录");
    this->setFixedSize(450, 500);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint);

    createUI();

    // 初始化控件状态
    m_widDispatcher->setHidden(true);
    m_widHandler->setHidden(true);
    m_cboDispatchers->setEnabled(false);
}

Login::~Login() = default;

void Login::createUI()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setContentsMargins(0, 0, 0, 0);

    buildLoginTab();
    buildRegisterTab();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_tabWidget);

    this->setLayout(mainLayout);
}

// 核心改造：登录标签页（同行/顶部图片/忘记密码/紧凑布局）
void Login::buildLoginTab()
{
    m_widLogin = new QWidget;
    QVBoxLayout *loginLayout = new QVBoxLayout(m_widLogin);
    // 登录页内边距更小，控件更靠拢
    loginLayout->setContentsMargins(30, 20, 30, 20);
    loginLayout->setSpacing(LOGIN_SPACING);
    // 登录页整体居中对齐（控件不会撑满，更美观）
    loginLayout->setAlignment(Qt::AlignCenter);

    /********** 1. 顶部居中图片（你自己改这里的资源路径！）**********/
    QLabel *labLogo = new QLabel;
    // ========== 你填写自己的资源相对路径，例：":/res/images/login_logo.png" 或 "./images/logo.png" ==========
    labLogo->setPixmap(QPixmap(":/sound/login.jpg").scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    labLogo->setScaledContents(false); // 保持图片比例，不拉伸
    labLogo->setAlignment(Qt::AlignCenter); // 图片居中
    labLogo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    loginLayout->addWidget(labLogo);
    // 图片和下方控件加个小间距
    loginLayout->addSpacing(5);

    /********** 2. 用户名：文字+输入框 同行排布 **********/
    QWidget *userRowWid = new QWidget;
    QHBoxLayout *userRowLayout = new QHBoxLayout(userRowWid);
    userRowLayout->setContentsMargins(0, 0, 0, 0);
    userRowLayout->setSpacing(SMALL_SPACING);
    m_labLoginUser = new QLabel("用户名：");
    m_labLoginUser->setFixedWidth(60); // 文字固定宽度，对齐更整齐
    m_leLoginUser = new QLineEdit;
    m_leLoginUser->setPlaceholderText("请输入登录用户名");
    m_leLoginUser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 输入框占满剩余空间
    userRowLayout->addWidget(m_labLoginUser);
    userRowLayout->addWidget(m_leLoginUser);
    loginLayout->addWidget(userRowWid);

    /********** 3. 密码：文字+输入框 同行排布 **********/
    QWidget *pwdRowWid = new QWidget;
    QHBoxLayout *pwdRowLayout = new QHBoxLayout(pwdRowWid);
    pwdRowLayout->setContentsMargins(0, 0, 0, 0);
    pwdRowLayout->setSpacing(SMALL_SPACING);
    m_labLoginPwd = new QLabel("密码：");
    m_labLoginPwd->setFixedWidth(60); // 和用户名文字同宽，对齐
    m_leLoginPwd = new QLineEdit;
    m_leLoginPwd->setPlaceholderText("请输入密码");
    m_leLoginPwd->setEchoMode(QLineEdit::Password);
    m_leLoginPwd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pwdRowLayout->addWidget(m_labLoginPwd);
    pwdRowLayout->addWidget(m_leLoginPwd);
    loginLayout->addWidget(pwdRowWid);

    /********** 4. 忘记密码（带下划线、靠右、可点击）**********/
    // 修复核心：用布局包裹按钮实现靠右（QPushButton无setAlignment）
    QWidget *forgetPwdWid = new QWidget;
    QHBoxLayout *forgetPwdLayout = new QHBoxLayout(forgetPwdWid);
    forgetPwdLayout->setContentsMargins(0, 0, 0, 0); // 无内边距，不占多余空间
    forgetPwdLayout->setSpacing(0);

    m_btnForgetPwd = new QPushButton("忘记密码");
    // 样式：下划线+蓝色+扁平无边框
    m_btnForgetPwd->setStyleSheet("QPushButton{color: #1890ff; text-decoration: underline; border: none; background: transparent;}"
                                  "QPushButton:hover{color: #40a9ff;}"); // 鼠标悬浮变浅蓝，更友好
    m_btnForgetPwd->setCursor(Qt::PointingHandCursor); // 鼠标变手型
    m_btnForgetPwd->setFlat(true); // 扁平样式，配合透明背景

    // 布局设置靠右（替代错误的setAlignment）
    forgetPwdLayout->addWidget(m_btnForgetPwd);
    forgetPwdLayout->setAlignment(Qt::AlignRight);

    loginLayout->addWidget(forgetPwdWid);
    // 忘记密码点击信号（留空槽，你后续实现逻辑即可）
    connect(m_btnForgetPwd, &QPushButton::clicked, this, &Login::on_btnForgetPwd_clicked);

    /********** 5. 登录/取消 按钮组（保持原样式，更紧凑）**********/
    m_widLoginBtn = new QWidget;
    QHBoxLayout *loginBtnLayout = new QHBoxLayout(m_widLoginBtn);
    loginBtnLayout->setSpacing(SMALL_SPACING);
    loginBtnLayout->setContentsMargins(0, 0, 0, 0);
    m_btnLogin = new QPushButton("登录");
    m_btnLogin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_btnLoginCancel = new QPushButton("取消");
    m_btnLoginCancel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    loginBtnLayout->addWidget(m_btnLogin);
    loginBtnLayout->addWidget(m_btnLoginCancel);
    loginLayout->addWidget(m_widLoginBtn);

    // 添加标签页
    m_tabWidget->addTab(m_widLogin, "登录");

    // 原有信号槽
    connect(m_btnLogin, &QPushButton::clicked, this, &Login::on_btnLogin_clicked);
    connect(m_btnLoginCancel, &QPushButton::clicked, this, &Login::on_btnLoginCancel_clicked);
}

// 注册标签页（完全不变，保留之前的优化）
void Login::buildRegisterTab()
{
    m_widRegister = new QWidget;
    QVBoxLayout *regMainLayout = new QVBoxLayout(m_widRegister);
    regMainLayout->setContentsMargins(LAYOUT_MARGIN, LAYOUT_MARGIN, LAYOUT_MARGIN, LAYOUT_MARGIN);
    regMainLayout->setSpacing(LAYOUT_SPACING);

    // 1. 通用注册项
    m_labRegUser = new QLabel("用户名：");
    m_leRegUser = new QLineEdit;
    m_leRegUser->setPlaceholderText("请设置登录用户名（全局唯一）");
    regMainLayout->addWidget(m_labRegUser);
    regMainLayout->addWidget(m_leRegUser);

    m_labRegPwd = new QLabel("密码：");
    m_leRegPwd = new QLineEdit;
    m_leRegPwd->setPlaceholderText("请设置密码（不少于6位）");
    m_leRegPwd->setEchoMode(QLineEdit::Password);
    regMainLayout->addWidget(m_labRegPwd);
    regMainLayout->addWidget(m_leRegPwd);

    m_labRegPwdConfirm = new QLabel("确认密码：");
    m_leRegPwdConfirm = new QLineEdit;
    m_leRegPwdConfirm->setPlaceholderText("请再次输入密码");
    m_leRegPwdConfirm->setEchoMode(QLineEdit::Password);
    regMainLayout->addWidget(m_labRegPwdConfirm);
    regMainLayout->addWidget(m_leRegPwdConfirm);

    m_labRegPhone = new QLabel("手机号：");
    m_leRegPhone = new QLineEdit;
    m_leRegPhone->setPlaceholderText("选填，用于系统通知");
    regMainLayout->addWidget(m_labRegPhone);
    regMainLayout->addWidget(m_leRegPhone);

    // 2. 身份选择组：设置大小策略，防止被压缩
    m_gboxRole = new QGroupBox("身份选择");
    m_gboxRole->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 固定高度，不被挤压
    QHBoxLayout *roleLayout = new QHBoxLayout(m_gboxRole);
    roleLayout->setSpacing(LAYOUT_SPACING);
    roleLayout->setContentsMargins(SMALL_SPACING, SMALL_SPACING, SMALL_SPACING, SMALL_SPACING);
    m_rbtnDispatcher = new QRadioButton("指挥调度员");
    m_rbtnHandler = new QRadioButton("现场处置员");
    m_btnGroupRole = new QButtonGroup(this);
    m_btnGroupRole->addButton(m_rbtnDispatcher);
    m_btnGroupRole->addButton(m_rbtnHandler);
    roleLayout->addWidget(m_rbtnDispatcher);
    roleLayout->addWidget(m_rbtnHandler);
    regMainLayout->addWidget(m_gboxRole);

    // 3. 指挥调度员专属：设置自适应大小策略
    m_widDispatcher = new QWidget;
    m_widDispatcher->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *dispLayout = new QVBoxLayout(m_widDispatcher);
    dispLayout->setSpacing(SMALL_SPACING);
    dispLayout->setContentsMargins(0, 0, 0, 0);
    m_labDeptCreate = new QLabel("所属单位：");
    m_leDeptCreate = new QLineEdit;
    m_leDeptCreate->setPlaceholderText("请输入单位名称（全局唯一）");
    dispLayout->addWidget(m_labDeptCreate);
    dispLayout->addWidget(m_leDeptCreate);
    regMainLayout->addWidget(m_widDispatcher);

    // 4. 现场处置员专属：设置自适应大小策略
    m_widHandler = new QWidget;
    m_widHandler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *handlerLayout = new QVBoxLayout(m_widHandler);
    handlerLayout->setSpacing(SMALL_SPACING);
    handlerLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *searchDeptWid = new QWidget;
    QHBoxLayout *searchDeptLayout = new QHBoxLayout(searchDeptWid);
    searchDeptLayout->setSpacing(SMALL_SPACING);
    searchDeptLayout->setContentsMargins(0, 0, 0, 0);
    m_labDeptSearch = new QLabel("搜索单位：");
    m_leDeptSearch = new QLineEdit;
    m_leDeptSearch->setPlaceholderText("请输入单位名称关键词");
    m_btnSearchDept = new QPushButton("搜索单位");
    m_btnSearchDept->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_btnSearchDept->setFixedWidth(80);
    searchDeptLayout->addWidget(m_labDeptSearch);
    searchDeptLayout->addWidget(m_leDeptSearch);
    searchDeptLayout->addWidget(m_btnSearchDept);

    m_labDispatcherSel = new QLabel("选择调度员：");
    m_cboDispatchers = new QComboBox;
    m_cboDispatchers->addItem("请先搜索单位", -1);
    m_cboDispatchers->setItemData(0, QColor(160, 160, 160), Qt::ForegroundRole);
    m_cboDispatchers->setCurrentIndex(0);

    handlerLayout->addWidget(searchDeptWid);
    handlerLayout->addWidget(m_labDispatcherSel);
    handlerLayout->addWidget(m_cboDispatchers);
    regMainLayout->addWidget(m_widHandler);

    // 5. 注册按钮组：固定在底部，不被压缩
    m_widRegBtn = new QWidget;
    m_widRegBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QHBoxLayout *regBtnLayout = new QHBoxLayout(m_widRegBtn);
    regBtnLayout->setSpacing(SMALL_SPACING);
    regBtnLayout->setContentsMargins(0, 0, 0, 0);
    m_btnRegister = new QPushButton("注册");
    m_btnRegister->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_btnRegReset = new QPushButton("重置");
    m_btnRegReset->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    regBtnLayout->addWidget(m_btnRegister);
    regBtnLayout->addWidget(m_btnRegReset);
    regMainLayout->addWidget(m_widRegBtn);

    m_tabWidget->addTab(m_widRegister, "注册");

    connect(m_rbtnDispatcher, &QRadioButton::toggled, this, &Login::on_rbtnDispatcher_toggled);
    connect(m_btnSearchDept, &QPushButton::clicked, this, &Login::on_btnSearchDept_clicked);
    connect(m_btnRegister, &QPushButton::clicked, this, &Login::on_btnRegister_clicked);
    connect(m_btnRegReset, &QPushButton::clicked, this, &Login::on_btnRegReset_clicked);

    connect(m_rbtnHandler, &QRadioButton::toggled, this, [=](bool checked) {
        if (checked) {
            m_widHandler->setHidden(false);
            m_widDispatcher->setHidden(true);
            m_selectedUnitId = 0;
            m_selectedDispatcherId = 0;
            m_leDeptSearch->clear();
            m_cboDispatchers->clear();
            m_cboDispatchers->addItem("请先搜索单位", -1);
            m_cboDispatchers->setItemData(0, QColor(160, 160, 160), Qt::ForegroundRole);
            m_cboDispatchers->setEnabled(false);
        }
    });
}

// 以下所有槽函数+私有方法 完全保留原逻辑，仅新增【忘记密码】空槽函数
void Login::on_btnLogin_clicked()
{
    QString username = m_leLoginUser->text().trimmed();
    QString password = m_leLoginPwd->text().trimmed();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "输入错误", "用户名和密码不能为空！");
        return;
    }

    AuthUser loginUser;
    QString errMsg;
    bool loginOk = UserAuth::login(username, password, &loginUser, &errMsg);

    if (loginOk)
    {
        QMessageBox::information(this, "登录成功",
                                 QString("欢迎%1：%2\n所属单位：%3")
                                 .arg(loginUser.role).arg(loginUser.username)
                                 .arg(loginUser.unit.isEmpty() ? "无" : loginUser.unit));
        this->accept();
    }
    else
    {
        QMessageBox::critical(this, "登录失败", errMsg);
        m_leLoginPwd->clear();
    }
}

void Login::on_btnLoginCancel_clicked()
{
    this->reject();
}

// 新增：忘记密码点击槽函数（你后续实现密码找回逻辑即可）
void Login::on_btnForgetPwd_clicked()
{
    // 暂留空，你可以加弹窗/跳转到密码找回页面
    QMessageBox::information(this, "提示", "密码找回功能正在开发中！");
}

void Login::on_rbtnDispatcher_toggled(bool checked)
{
    if (checked)
    {
        m_widDispatcher->setHidden(false);
        m_widHandler->setHidden(true);
        m_selectedUnitId = 0;
        m_selectedDispatcherId = 0;
        m_leDeptSearch->clear();
        m_cboDispatchers->clear();
        m_cboDispatchers->addItem("请先搜索单位", -1);
        m_cboDispatchers->setItemData(0, QColor(160, 160, 160), Qt::ForegroundRole);
        m_cboDispatchers->setEnabled(false);
    }
    else
    {
        m_widDispatcher->setHidden(true);
    }
}

void Login::on_btnSearchDept_clicked()
{
    QString deptKey = m_leDeptSearch->text().trimmed();
    if (deptKey.isEmpty())
    {
        QMessageBox::warning(this, "输入错误", "请输入单位名称关键词！");
        return;
    }

    QList<UnitInfo> searchDepts;
    QString errMsg;
    bool searchOk = UserAuth::searchUnits(deptKey, 20, &searchDepts, &errMsg);
    if (!searchOk)
    {
        QMessageBox::critical(this, "搜索失败", errMsg);
        return;
    }
    if (searchDepts.isEmpty())
    {
        QMessageBox::information(this, "搜索结果", "未找到该单位，请确认名称！");
        m_cboDispatchers->clear();
        m_cboDispatchers->addItem("请先搜索单位", -1);
        m_cboDispatchers->setItemData(0, QColor(160, 160, 160), Qt::ForegroundRole);
        m_cboDispatchers->setEnabled(false);
        m_selectedUnitId = 0;
        return;
    }

    m_selectedUnitId = searchDepts.first().id;
    QString deptName = searchDepts.first().name;
    QMessageBox::information(this, "搜索成功", QString("找到单位：%1，正在加载调度员...").arg(deptName));

    QList<DispatcherInfo> searchDispatchers;
    bool searchDispOk = UserAuth::searchDispatchersByUnit(m_selectedUnitId, "", 20, &searchDispatchers, &errMsg);
    if (!searchDispOk || searchDispatchers.isEmpty())
    {
        QMessageBox::warning(this, "无调度员", QString("单位【%1】下暂无调度员，无法注册！").arg(deptName));
        m_cboDispatchers->clear();
        m_cboDispatchers->addItem("请先搜索单位", -1);
        m_cboDispatchers->setItemData(0, QColor(160, 160, 160), Qt::ForegroundRole);
        m_cboDispatchers->setEnabled(false);
        m_selectedUnitId = 0;
        return;
    }

    m_cboDispatchers->clear();
    m_cboDispatchers->setEnabled(true);
    for (const DispatcherInfo &disp : searchDispatchers)
    {
        m_cboDispatchers->addItem(QString("%1（%2）").arg(disp.username).arg(disp.phone.isEmpty() ? "无手机号" : disp.phone), disp.id);
    }
    m_cboDispatchers->setCurrentIndex(0);
    m_selectedDispatcherId = searchDispatchers.first().id;
}

void Login::on_btnRegister_clicked()
{
    if (!checkRegForm()) return;

    QString username = m_leRegUser->text().trimmed();
    QString password = m_leRegPwd->text().trimmed();
    QString phone = m_leRegPhone->text().trimmed();
    QString errMsg;
    bool regOk = false;
    qint64 newUserId = 0;

    if (m_rbtnDispatcher->isChecked())
    {
        QString deptName = m_leDeptCreate->text().trimmed();
        qint64 createDeptId = 0;
        bool createDeptOk = UserAuth::createUnit(deptName, &createDeptId, &errMsg);
        if (!createDeptOk)
        {
            QMessageBox::critical(this, "单位创建失败", errMsg);
            return;
        }
        regOk = UserAuth::registerDispatcherWithUnitId(
                    username, password, phone,
                    "指挥调度员", createDeptId,
                    &newUserId, &errMsg);
    }
    else if (m_rbtnHandler->isChecked())
    {
        m_selectedDispatcherId = m_cboDispatchers->currentData().toLongLong();
        if (m_selectedDispatcherId <= 0)
        {
            QMessageBox::warning(this, "选择错误", "请先搜索单位并选择有效调度员！");
            return;
        }
        regOk = UserAuth::registerHandlerWithDispatcherId(
                    username, password, phone,
                    m_selectedDispatcherId,
                    &newUserId, &errMsg);
    }
    else
    {
        QMessageBox::warning(this, "选择错误", "请选择注册身份（指挥调度员/现场处置员）！");
        return;
    }

    if (regOk)
    {
        QMessageBox::information(this, "注册成功",
                                 QString("注册成功！\n用户ID：%1\n请返回登录标签页登录.").arg(newUserId));
        resetRegForm();
    }
    else
    {
        QMessageBox::critical(this, "注册失败", errMsg);
    }
}

void Login::on_btnRegReset_clicked()
{
    resetRegForm();
}

bool Login::checkRegForm()
{
    QString username = m_leRegUser->text().trimmed();
    QString pwd = m_leRegPwd->text().trimmed();
    QString pwdConfirm = m_leRegPwdConfirm->text().trimmed();
    QString deptCreate = m_leDeptCreate->text().trimmed();

    if (username.isEmpty())
    {
        QMessageBox::warning(this, "输入错误", "用户名不能为空！");
        m_leRegUser->setFocus();
        return false;
    }
    if (pwd.isEmpty() || pwd.length() < 6)
    {
        QMessageBox::warning(this, "输入错误", "密码不能为空且长度不能少于6位！");
        m_leRegPwd->setFocus();
        return false;
    }
    if (pwd != pwdConfirm)
    {
        QMessageBox::warning(this, "输入错误", "两次输入的密码不一致！");
        m_leRegPwdConfirm->setFocus();
        return false;
    }
    if (m_rbtnDispatcher->isChecked() && deptCreate.isEmpty())
    {
        QMessageBox::warning(this, "输入错误", "指挥调度员必须填写单位名称！");
        m_leDeptCreate->setFocus();
        return false;
    }

    return true;
}

void Login::resetRegForm()
{
    m_leRegUser->clear();
    m_leRegPwd->clear();
    m_leRegPwdConfirm->clear();
    m_leRegPhone->clear();
    m_leDeptCreate->clear();
    m_leDeptSearch->clear();
    m_cboDispatchers->clear();
    m_cboDispatchers->addItem("请先搜索单位", -1);
    m_cboDispatchers->setItemData(0, QColor(160, 160, 160), Qt::ForegroundRole);
    m_cboDispatchers->setEnabled(false);
    m_btnGroupRole->setExclusive(false);
    m_rbtnDispatcher->setChecked(false);
    m_rbtnHandler->setChecked(false);
    m_btnGroupRole->setExclusive(true);
    m_widDispatcher->setHidden(true);
    m_widHandler->setHidden(true);
    m_selectedUnitId = 0;
    m_selectedDispatcherId = 0;
    m_leRegUser->setFocus();
}
