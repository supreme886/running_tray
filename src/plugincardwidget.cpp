#include "plugincardwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

PluginCardWidget::PluginCardWidget(QWidget *parent) 
    : QFrame(parent), isRunning(false) {
    // 设置卡片样式
    setFrameShape(QFrame::StyledPanel);
    setStyleSheet("QFrame { border: 1px solid #ccc; border-radius: 8px; padding: 15px; background-color: #f5f5f5; }");
    setFixedSize(200, 250);

    // 创建布局
    QVBoxLayout *cardLayout = new QVBoxLayout(this);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(15);


    QHBoxLayout *iconLayout = new QHBoxLayout(this);
    iconLayout->setAlignment(Qt::AlignCenter);

    // 插件图标
    iconLabel = new QLabel();
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(80, 80);
    iconLabel->setStyleSheet("background-color: glay; border-radius: 40px; padding: 5px;");
    iconLayout->addWidget(iconLabel);
    cardLayout->addLayout(iconLayout);

    // 插件名称
    nameLabel = new QLabel();
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    cardLayout->addWidget(nameLabel);

    // 启动/停止按钮
    controlBtn = new QPushButton("Start");
    controlBtn->setStyleSheet("padding: 8px; font-size: 14px;");
    cardLayout->addWidget(controlBtn);

    // 连接按钮信号
    connect(controlBtn, &QPushButton::clicked, this, &PluginCardWidget::onControlButtonClicked);
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
