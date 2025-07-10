#include "plugincardwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QApplication>
#include <QStyle>
#include <QPixmap>
#include <QColor>
#include <QApplication>
#include <QStyle>
#include <QPixmap>
#include <QColor>

PluginCardWidget::PluginCardWidget(QWidget *parent) 
    : QFrame(parent), isRunning(false) {
    setFrameShape(QFrame::StyledPanel);
    setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);
    setFixedSize(200, 250);

    // 创建布局
    QVBoxLayout *cardLayout = new QVBoxLayout(this);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setContentsMargins(0, 10, 0, 0);
    cardLayout->setSpacing(10);
    cardLayout->setContentsMargins(0, 10, 0, 0);
    cardLayout->setSpacing(10);


    QHBoxLayout *iconLayout = new QHBoxLayout;
    iconLayout->setAlignment(Qt::AlignCenter);

    // 插件图标
    iconLabel = new QLabel(this);
    iconLabel->setObjectName("iconLabel"); 
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(80, 80);
    iconLayout->addWidget(iconLabel);
    cardLayout->addLayout(iconLayout);

    // 插件名称
    nameLabel = new QLabel(this);
    nameLabel->setObjectName("nameLabel");
    nameLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(nameLabel);

    // 新增：插件描述
    descLabel = new QLabel(this);
    descLabel->setObjectName("descLabel");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);  // 自动换行
    descLabel->setMaximumWidth(180);  // 限制宽度
    cardLayout->addWidget(descLabel);
    
    // 控制按钮
    controlBtn = new QPushButton("Start");
    controlBtn->setObjectName("controlBtn");
    controlBtn->setFixedSize(width(), 40);
    cardLayout->addStretch(1);
    controlBtn->setObjectName("controlBtn");
    controlBtn->setFixedSize(width(), 40);
    cardLayout->addStretch(1);
    cardLayout->addWidget(controlBtn);
    
    // 设置按钮
    settingsBtn = new QPushButton(this);
    settingsBtn->setFixedSize(24, 24);
    settingsBtn->setObjectName("settingsBtn");
    settingsBtn->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowRight));
    settingsBtn->setToolTip("settings");
    settingsBtn->setVisible(false);
    settingsBtn->move(width() - settingsBtn->width(), 0);
    
    // 设置按钮
    settingsBtn = new QPushButton(this);
    settingsBtn->setFixedSize(24, 24);
    settingsBtn->setObjectName("settingsBtn");
    settingsBtn->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowRight));
    settingsBtn->setToolTip("settings");
    settingsBtn->setVisible(false);
    settingsBtn->move(width() - settingsBtn->width(), 0);

    // 连接按钮信号
    connect(controlBtn, &QPushButton::clicked, this, &PluginCardWidget::onControlButtonClicked);
    connect(settingsBtn, &QPushButton::clicked, this, &PluginCardWidget::settingsClicked);
    connect(settingsBtn, &QPushButton::clicked, this, &PluginCardWidget::settingsClicked);
}

void PluginCardWidget::setPluginName(const QString &name) {
    nameLabel->setText(name);
}

void PluginCardWidget::setPluginIcon(const QIcon &icon) {
    iconLabel->setPixmap(icon.pixmap(90, 90));
}

void PluginCardWidget::setRunningState(bool running) {
    isRunning = running;
    controlBtn->setText(running ? "Stop" : "Start");
}

void PluginCardWidget::onControlButtonClicked() {
    isRunning = !isRunning;
    controlBtn->setText(isRunning ? "Stop" : "Start");
    emit controlClicked(isRunning);
}

// 新增：设置描述信息的方法
void PluginCardWidget::setPluginDescription(const QString &description) {
    descLabel->setText(description);
}

void PluginCardWidget::setHasSettings(bool hasSettings) {
    settingsBtn->setVisible(hasSettings);
}


// 新增：EmptyCardWidget实现
EmptyCardWidget::EmptyCardWidget(QWidget *parent) 
    : PluginCardWidget(parent) {
    // 设置"Coming Soon"文本
    setPluginName("Coming Soon");
    setPluginDescription("More features coming soon...");
    
    // 禁用控制按钮并修改文本
    controlBtn->setEnabled(false);
    controlBtn->setText("Coming Soon");
    
    // 隐藏设置按钮
    settingsBtn->setVisible(false);
    
    // 创建灰色占位符图标
    QPixmap placeholder(80, 80);
    placeholder.fill(QColor("#d4eaf7")); // 灰色背景
    setPluginIcon(QIcon(placeholder));
}
