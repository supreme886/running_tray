#include "pluginsettingswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>

PluginSettingsWidget::PluginSettingsWidget(ITrayLoadPlugin* plugin, QWidget *parent)
    : QWidget(parent), m_plugin(plugin), m_settingsWidget(nullptr) {
    if (!m_plugin) {
        setVisible(false);
        return;
    }

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* titleLayout = new QHBoxLayout;

    QPushButton* backBtn = new QPushButton(this);
    backBtn->setObjectName("settingsBtn");
    backBtn->setFixedSize(24, 24);
    backBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    backBtn->setToolTip("Back");
    connect(backBtn, &QPushButton::clicked, this, &PluginSettingsWidget::cancelRequested);
    titleLayout->addWidget(backBtn);
    
    QLabel* titleLabel = new QLabel(tr("Plugin Settings - %1").arg(m_plugin->name()));
    titleLabel->setObjectName("settingsTitle");
    titleLayout->addStretch(1);
    titleLayout->addWidget(titleLabel);
    
    mainLayout->addLayout(titleLayout);
    
    m_settingsWidget = m_plugin->createSettingsWidget();
    if (m_settingsWidget) {
        // 创建滚动区域
        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidget(m_settingsWidget);
        scrollArea->setWidgetResizable(true);  // 允许部件调整大小
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 禁用水平滚动条
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);     // 垂直滚动条按需显示
        
        // 将滚动区域添加到布局
        mainLayout->addWidget(scrollArea);
    }
    mainLayout->addStretch(1);

    // 创建按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    QPushButton* cancelBtn = new QPushButton("取消");
    cancelBtn->hide();
    connect(cancelBtn, &QPushButton::clicked, this, &PluginSettingsWidget::cancelRequested);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(cancelBtn);
    
    QPushButton* saveBtn = new QPushButton("保存");
    saveBtn->hide();
    saveBtn->setObjectName("saveSettingsBtn");
    connect(saveBtn, &QPushButton::clicked, this, &PluginSettingsWidget::saveRequested);
    buttonLayout->addWidget(saveBtn);
    
    mainLayout->addLayout(buttonLayout);
}

PluginSettingsWidget::~PluginSettingsWidget() {
    // 不需要手动删除m_settingsWidget，Qt的父对象机制会自动处理
}
