#include "appsettingswidget.h"
#include "autostartmanager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>

AppSettingsWidget::AppSettingsWidget(QWidget *parent) : QWidget(parent) {
    settingsLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("App Settings", this);
    titleLabel->setObjectName("settingsTitle");
    settingsLayout->addWidget(titleLabel);
    settingsLayout->addStretch(1);
    
    autoStartCheckBox = new QCheckBox("Auto-start on boot", this);
    autoStartCheckBox->setChecked(AutoStartManager::instance().isAutoStartEnabled());
    connect(autoStartCheckBox, &QCheckBox::toggled, [](bool checked) {
        AutoStartManager::instance().setAutoStart(checked);
    });
    settingsLayout->addWidget(autoStartCheckBox);
    
    saveBtn = new QPushButton("Save", this);
    saveBtn->setObjectName("saveSettingsBtn");
    connect(saveBtn, &QPushButton::clicked, this, &AppSettingsWidget::saveRequested);
    settingsLayout->addWidget(saveBtn);
}
