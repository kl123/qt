#include "login.h"
// Qt核心库
#include <QCoreApplication>
#include <QSizePolicy>
#include <QButtonGroup>
#include <QVariant>
#include <QColor>
#include <QMessageBox>
#include <QStyle> // 图片缩放用
#include"mainwindow.h"

// 全局布局常量（调整为更宽松的适配值，解决显示不全）
const int LAYOUT_MARGIN = 12;     // 外边缘间距（缩小，增加内部空间）
const int LAYOUT_SPACING = 8;     // 注册页控件间距（缩小）
const int SMALL_SPACING = 5;      // 小间距（同行/按钮）
const int LOGIN_SPACING = 6;      // 登录页专属更小间距

Login::Login(QWidget *parent) : QDialog(parent)
{
    // 窗口基础属性：加宽加高，确保内容完整显示
    this->setWindowTitle("用户登录");
    this->setFixedSize(500, 600); // 原450x500 → 500x600，增加显示空间
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint);
    // 窗口整体背景美化
    this->setStyleSheet("QDialog { background-color: #F5F7FA; }");

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
    // TabWidget美化 + 调整内边距，避免内容被遮挡
    m_tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #E5E6EB;
            background-color: white;
            border-radius: 8px;
            margin: 8px; /* 缩小外边距，增加内部可用空间 */
        }
        QTabBar::tab {
            padding: 8px 18px; /* 缩小tab内边距，避免文字溢出 */
            margin-right: 2px;
            border-radius: 4px 4px 0 0;
            font-size: 14px;
            color: #666666;
            min-width: 80px; /* 给tab设置最小宽度，避免文字挤在一起 */
        }
        QTabBar::tab:selected {
            background-color: white;
            color: #2F54EB;
            font-weight: bold;
        }
        QTabBar::tab:hover:!selected {
            background-color: #F0F2F5;
        }
    )");
    m_tabWidget->setContentsMargins(0, 0, 0, 0);

    buildLoginTab();
    buildRegisterTab();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5); // 缩小整体布局内边距
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_tabWidget);

    this->setLayout(mainLayout);
}

// 核心改造：登录标签页（修复显示不全 + 保留美化）
void Login::buildLoginTab()
{
    m_widLogin = new QWidget;
    // 登录页背景美化
    m_widLogin->setStyleSheet("QWidget { background-color: white; }");
    QVBoxLayout *loginLayout = new QVBoxLayout(m_widLogin);
    // 调整登录页内边距，避免内容溢出
    loginLayout->setContentsMargins(20, 15, 20, 15); // 原30,20 → 20,15
    loginLayout->setSpacing(LOGIN_SPACING);
    // 登录页整体居中对齐（控件不会撑满，更美观）
    loginLayout->setAlignment(Qt::AlignCenter);

    /********** 1. 顶部居中图片（美化版 + 调整尺寸）**********/
    QLabel *labLogo = new QLabel;
    // 缩小图片尺寸，避免占太多空间
    labLogo->setPixmap(QPixmap(":/sound/login.jpg").scaled(150, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    labLogo->setScaledContents(false); // 保持图片比例，不拉伸
    labLogo->setAlignment(Qt::AlignCenter); // 图片居中
    labLogo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // Logo容器美化（缩小内边距）
    labLogo->setStyleSheet(R"(
        QLabel {
            border-radius: 8px;
            padding: 8px; /* 原10px → 8px */
            margin-bottom: 8px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
    )");
    loginLayout->addWidget(labLogo);
    // 图片和下方控件加个小间距
    loginLayout->addSpacing(10);

    /********** 2. 用户名：文字+输入框 同行排布（调整宽度）**********/
    QWidget *userRowWid = new QWidget;
    QHBoxLayout *userRowLayout = new QHBoxLayout(userRowWid);
    userRowLayout->setContentsMargins(0, 0, 0, 0);
    userRowLayout->setSpacing(SMALL_SPACING);
    m_labLoginUser = new QLabel("用户名：");
    m_labLoginUser->setFixedWidth(55); // 原60 → 55，缩小固定宽度
    // 标签文字美化
    m_labLoginUser->setStyleSheet("QLabel { color: #333333; font-size: 14px; }");

    m_leLoginUser = new QLineEdit;
    m_leLoginUser->setPlaceholderText("请输入登录用户名");
    m_leLoginUser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 输入框美化（调整内边距，避免文字被遮挡）
    m_leLoginUser->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            padding: 7px 10px; /* 原8px 12px → 7px 10px */
            font-size: 14px;
            color: #333333;
            background-color: #FFFFFF;
            min-height: 30px; /* 保证输入框高度足够 */
        }
        QLineEdit:focus {
            border: 1px solid #2F54EB;
            box-shadow: 0 0 0 2px rgba(47, 84, 235, 0.1);
        }
        QLineEdit::placeholder {
            color: #909399;
        }
    )");

    userRowLayout->addWidget(m_labLoginUser);
    userRowLayout->addWidget(m_leLoginUser);
    loginLayout->addWidget(userRowWid);

    /********** 3. 密码：文字+输入框 同行排布（调整宽度）**********/
    QWidget *pwdRowWid = new QWidget;
    QHBoxLayout *pwdRowLayout = new QHBoxLayout(pwdRowWid);
    pwdRowLayout->setContentsMargins(0, 0, 0, 0);
    pwdRowLayout->setSpacing(SMALL_SPACING);
    m_labLoginPwd = new QLabel("密码：");
    m_labLoginPwd->setFixedWidth(55); // 原60 → 55
    m_labLoginPwd->setStyleSheet("QLabel { color: #333333; font-size: 14px; }");

    m_leLoginPwd = new QLineEdit;
    m_leLoginPwd->setPlaceholderText("请输入密码");
    m_leLoginPwd->setEchoMode(QLineEdit::Password);
    m_leLoginPwd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 输入框美化（同用户名输入框）
    m_leLoginPwd->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            padding: 7px 10px;
            font-size: 14px;
            color: #333333;
            background-color: #FFFFFF;
            min-height: 30px;
        }
        QLineEdit:focus {
            border: 1px solid #2F54EB;
            box-shadow: 0 0 0 2px rgba(47, 84, 235, 0.1);
        }
        QLineEdit::placeholder {
            color: #909399;
        }
    )");

    pwdRowLayout->addWidget(m_labLoginPwd);
    pwdRowLayout->addWidget(m_leLoginPwd);
    loginLayout->addWidget(pwdRowWid);

    /********** 4. 忘记密码（调整间距）**********/
    QWidget *forgetPwdWid = new QWidget;
    QHBoxLayout *forgetPwdLayout = new QHBoxLayout(forgetPwdWid);
    forgetPwdLayout->setContentsMargins(0, 0, 0, 0);
    forgetPwdLayout->setSpacing(0);

    m_btnForgetPwd = new QPushButton("忘记密码");
    // 忘记密码链接美化（调整字体大小）
    m_btnForgetPwd->setStyleSheet(R"(
        QPushButton {
            color: #2F54EB;
            text-decoration: underline;
            border: none;
            background: transparent;
            font-size: 12px; /* 原13px → 12px */
            padding: 0;
        }
        QPushButton:hover {
            color: #1D39C4;
        }
    )");
    m_btnForgetPwd->setCursor(Qt::PointingHandCursor);
    m_btnForgetPwd->setFlat(true);

    forgetPwdLayout->addWidget(m_btnForgetPwd);
    forgetPwdLayout->setAlignment(Qt::AlignRight);

    loginLayout->addWidget(forgetPwdWid);
    loginLayout->addSpacing(8); // 原10 → 8

    /********** 5. 登录/取消 按钮组（调整按钮内边距）**********/
    m_widLoginBtn = new QWidget;
    QHBoxLayout *loginBtnLayout = new QHBoxLayout(m_widLoginBtn);
    loginBtnLayout->setSpacing(SMALL_SPACING);
    loginBtnLayout->setContentsMargins(0, 0, 0, 0);

    m_btnLogin = new QPushButton("登录");
    m_btnLogin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 登录按钮美化（缩小内边距）
    m_btnLogin->setStyleSheet(R"(
        QPushButton {
            background-color: #2F54EB;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 8px 15px; /* 原10px 20px → 8px 15px */
            font-size: 14px;
            min-height: 32px; /* 保证按钮高度 */
        }
        QPushButton:hover {
            background-color: #1D39C4;
        }
        QPushButton:pressed {
            background-color: #182E99;
        }
        QPushButton:disabled {
            background-color: #C0C4CC;
            color: #FFFFFF;
        }
    )");

    m_btnLoginCancel = new QPushButton("取消");
    m_btnLoginCancel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 取消按钮美化（同登录按钮）
    m_btnLoginCancel->setStyleSheet(R"(
        QPushButton {
            background-color: #F0F2F5;
            color: #333333;
            border: none;
            border-radius: 4px;
            padding: 8px 15px;
            font-size: 14px;
            min-height: 32px;
        }
        QPushButton:hover {
            background-color: #E5E6EB;
        }
        QPushButton:pressed {
            background-color: #C9CDD4;
        }
    )");

    loginBtnLayout->addWidget(m_btnLogin);
    loginBtnLayout->addWidget(m_btnLoginCancel);
    loginLayout->addWidget(m_widLoginBtn);

    // 添加标签页
    m_tabWidget->addTab(m_widLogin, "登录");

    // 原有信号槽
    connect(m_btnLogin, &QPushButton::clicked, this, &Login::on_btnLogin_clicked);
    connect(m_btnLoginCancel, &QPushButton::clicked, this, &Login::on_btnLoginCancel_clicked);
}

// 注册标签页（核心修复显示不全 + 保留美化）
void Login::buildRegisterTab()
{
    m_widRegister = new QWidget;
    m_widRegister->setStyleSheet("QWidget { background-color: white; }");
    QVBoxLayout *regMainLayout = new QVBoxLayout(m_widRegister);
    // 调整注册页内边距，增加可用空间
    regMainLayout->setContentsMargins(LAYOUT_MARGIN, LAYOUT_MARGIN, LAYOUT_MARGIN, LAYOUT_MARGIN);
    regMainLayout->setSpacing(LAYOUT_SPACING);
    // 允许布局垂直滚动（兜底方案，避免内容溢出）
    regMainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // 统一的输入框样式（调整内边距）
    QString lineEditStyle = R"(
        QLineEdit {
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            padding: 7px 10px; /* 原8px 12px → 7px 10px */
            font-size: 14px;
            color: #333333;
            background-color: #FFFFFF;
            min-height: 30px;
        }
        QLineEdit:focus {
            border: 1px solid #2F54EB;
            box-shadow: 0 0 0 2px rgba(47, 84, 235, 0.1);
        }
        QLineEdit::placeholder {
            color: #909399;
        }
    )";

    // 统一的标签样式（缩小下边距）
    QString labelStyle = "QLabel { color: #333333; font-size: 14px; margin-bottom: 2px; }"; // 原4px → 2px

    // 1. 通用注册项（调整间距）
    m_labRegUser = new QLabel("用户名：");
    m_labRegUser->setStyleSheet(labelStyle);
    m_leRegUser = new QLineEdit;
    m_leRegUser->setPlaceholderText("请设置登录用户名（全局唯一）");
    m_leRegUser->setStyleSheet(lineEditStyle);
    regMainLayout->addWidget(m_labRegUser);
    regMainLayout->addWidget(m_leRegUser);

    m_labRegPwd = new QLabel("密码：");
    m_labRegPwd->setStyleSheet(labelStyle);
    m_leRegPwd = new QLineEdit;
    m_leRegPwd->setPlaceholderText("请设置密码（不少于6位）");
    m_leRegPwd->setEchoMode(QLineEdit::Password);
    m_leRegPwd->setStyleSheet(lineEditStyle);
    regMainLayout->addWidget(m_labRegPwd);
    regMainLayout->addWidget(m_leRegPwd);

    m_labRegPwdConfirm = new QLabel("确认密码：");
    m_labRegPwdConfirm->setStyleSheet(labelStyle);
    m_leRegPwdConfirm = new QLineEdit;
    m_leRegPwdConfirm->setPlaceholderText("请再次输入密码");
    m_leRegPwdConfirm->setEchoMode(QLineEdit::Password);
    m_leRegPwdConfirm->setStyleSheet(lineEditStyle);
    regMainLayout->addWidget(m_labRegPwdConfirm);
    regMainLayout->addWidget(m_leRegPwdConfirm);

    m_labRegPhone = new QLabel("手机号：");
    m_labRegPhone->setStyleSheet(labelStyle);
    m_leRegPhone = new QLineEdit;
    m_leRegPhone->setPlaceholderText("选填，用于系统通知");
    m_leRegPhone->setStyleSheet(lineEditStyle);
    regMainLayout->addWidget(m_labRegPhone);
    regMainLayout->addWidget(m_leRegPhone);

    // 2. 身份选择组（调整内边距和间距）
    m_gboxRole = new QGroupBox("身份选择");
    m_gboxRole->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 分组框美化（缩小内边距）
    m_gboxRole->setStyleSheet(R"(
        QGroupBox {
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            margin-top: 8px; /* 原10px → 8px */
            padding-top: 12px; /* 原15px → 12px */
            font-size: 14px;
            color: #333333;
            background-color: #F8F9FA;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 8px; /* 原10px → 8px */
            padding: 0 4px 0 4px; /* 原5px → 4px */
            color: #2F54EB;
            font-weight: bold;
        }
    )");

    QHBoxLayout *roleLayout = new QHBoxLayout(m_gboxRole);
    roleLayout->setSpacing(LAYOUT_SPACING); // 原2倍 → 1倍
    roleLayout->setContentsMargins(SMALL_SPACING, SMALL_SPACING, SMALL_SPACING, SMALL_SPACING);

    // 单选按钮样式（调整字体大小）
    QString radioStyle = R"(
        QRadioButton {
            font-size: 13px; /* 原14px → 13px */
            color: #333333;
            padding: 3px 0; /* 原4px → 3px */
        }
        QRadioButton::indicator {
            width: 15px; /* 原16px → 15px */
            height: 15px;
            border-radius: 7px;
            border: 1px solid #DCDFE6;
            background-color: #FFFFFF;
        }
        QRadioButton::indicator:checked {
            background-color: #2F54EB;
            border: 1px solid #2F54EB;
        }
        QRadioButton::indicator:hover {
            border-color: #2F54EB;
        }
    )";

    m_rbtnDispatcher = new QRadioButton("指挥调度员");
    m_rbtnDispatcher->setStyleSheet(radioStyle);
    m_rbtnHandler = new QRadioButton("现场处置员");
    m_rbtnHandler->setStyleSheet(radioStyle);

    m_btnGroupRole = new QButtonGroup(this);
    m_btnGroupRole->addButton(m_rbtnDispatcher);
    m_btnGroupRole->addButton(m_rbtnHandler);

    roleLayout->addWidget(m_rbtnDispatcher);
    roleLayout->addWidget(m_rbtnHandler);
    regMainLayout->addWidget(m_gboxRole);

    // 3. 指挥调度员专属（调整间距）
    m_widDispatcher = new QWidget;
    m_widDispatcher->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *dispLayout = new QVBoxLayout(m_widDispatcher);
    dispLayout->setSpacing(SMALL_SPACING);
    dispLayout->setContentsMargins(0, 0, 0, 0);

    m_labDeptCreate = new QLabel("所属单位：");
    m_labDeptCreate->setStyleSheet(labelStyle);
    m_leDeptCreate = new QLineEdit;
    m_leDeptCreate->setPlaceholderText("请输入单位名称（全局唯一）");
    m_leDeptCreate->setStyleSheet(lineEditStyle);

    dispLayout->addWidget(m_labDeptCreate);
    dispLayout->addWidget(m_leDeptCreate);
    regMainLayout->addWidget(m_widDispatcher);

    // 4. 现场处置员专属（调整控件宽度和内边距）
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
    m_labDeptSearch->setStyleSheet(labelStyle);
    m_labDeptSearch->setFixedWidth(65); // 原70 → 65

    m_leDeptSearch = new QLineEdit;
    m_leDeptSearch->setPlaceholderText("请输入单位名称关键词");
    m_leDeptSearch->setStyleSheet(lineEditStyle);

    m_btnSearchDept = new QPushButton("搜索单位");
    m_btnSearchDept->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_btnSearchDept->setFixedWidth(75); // 原80 → 75
    // 搜索按钮美化（缩小内边距）
    m_btnSearchDept->setStyleSheet(R"(
        QPushButton {
            background-color: #2F54EB;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 7px 10px; /* 原8px 12px → 7px 10px */
            font-size: 13px; /* 原14px → 13px */
            min-height: 30px;
        }
        QPushButton:hover {
            background-color: #1D39C4;
        }
        QPushButton:pressed {
            background-color: #182E99;
        }
    )");

    searchDeptLayout->addWidget(m_labDeptSearch);
    searchDeptLayout->addWidget(m_leDeptSearch);
    searchDeptLayout->addWidget(m_btnSearchDept);

    m_labDispatcherSel = new QLabel("选择调度员：");
    m_labDispatcherSel->setStyleSheet(labelStyle);

    m_cboDispatchers = new QComboBox;
    m_cboDispatchers->addItem("请先搜索单位", -1);
    m_cboDispatchers->setItemData(0, QColor(160, 160, 160), Qt::ForegroundRole);
    m_cboDispatchers->setCurrentIndex(0);
    // 下拉框美化（调整内边距）
    m_cboDispatchers->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            padding: 7px 10px; /* 原8px 12px → 7px 10px */
            font-size: 14px;
            color: #333333;
            background-color: #FFFFFF;
            min-height: 30px;
        }
        QComboBox:focus {
            border: 1px solid #2F54EB;
            box-shadow: 0 0 0 2px rgba(47, 84, 235, 0.1);
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 18px; /* 原20px → 18px */
            border-left: none;
        }
        QComboBox::down-arrow {
            image: url(:/icons/arrow_down.png);
            width: 7px; /* 原8px → 7px */
            height: 7px;
        }
        QComboBox QAbstractItemView {
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            padding: 3px; /* 原4px → 3px */
            font-size: 13px; /* 原14px → 13px */
        }
    )");

    handlerLayout->addWidget(searchDeptWid);
    handlerLayout->addWidget(m_labDispatcherSel);
    handlerLayout->addWidget(m_cboDispatchers);
    regMainLayout->addWidget(m_widHandler);

    // 5. 注册按钮组（调整按钮内边距）
    m_widRegBtn = new QWidget;
    m_widRegBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QHBoxLayout *regBtnLayout = new QHBoxLayout(m_widRegBtn);
    regBtnLayout->setSpacing(SMALL_SPACING);
    regBtnLayout->setContentsMargins(0, 0, 0, 0);

    m_btnRegister = new QPushButton("注册");
    m_btnRegister->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 注册按钮美化（缩小内边距）
    m_btnRegister->setStyleSheet(R"(
        QPushButton {
            background-color: #2F54EB;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 8px 15px; /* 原10px 20px → 8px 15px */
            font-size: 14px;
            min-height: 32px;
        }
        QPushButton:hover {
            background-color: #1D39C4;
        }
        QPushButton:pressed {
            background-color: #182E99;
        }
        QPushButton:disabled {
            background-color: #C0C4CC;
            color: #FFFFFF;
        }
    )");

    m_btnRegReset = new QPushButton("重置");
    m_btnRegReset->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 重置按钮美化（同注册按钮）
    m_btnRegReset->setStyleSheet(R"(
        QPushButton {
            background-color: #F0F2F5;
            color: #333333;
            border: none;
            border-radius: 4px;
            padding: 8px 15px;
            font-size: 14px;
            min-height: 32px;
        }
        QPushButton:hover {
            background-color: #E5E6EB;
        }
        QPushButton:pressed {
            background-color: #C9CDD4;
        }
    )");

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

// 以下所有槽函数+私有方法 完全保留原逻辑
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
        // ========== 新增/修改部分 ==========
        // 1. 关闭登录窗口（保留 accept 也可以，accept 会触发 dialog 的 finished 信号）
        this->close();

        // 2. 创建并显示主界面（假设你的主界面类是 MainWindow，需提前包含头文件）
        MainWindow *mainWin = new MainWindow; // 主界面对象
        mainWin->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动释放内存
        mainWin->show(); // 显示主界面
        // ========== 结束 ==========
//        this->accept();

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
