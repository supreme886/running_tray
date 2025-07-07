#ifndef PLUGINSETTINGSWIDGET_H
#define PLUGINSETTINGSWIDGET_H

#include <QWidget>
#include "interface/itrayloadplugin.h"

class QVBoxLayout;
class QLabel;
class QPushButton;

class PluginSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit PluginSettingsWidget(ITrayLoadPlugin* plugin, QWidget *parent = nullptr);
    ~PluginSettingsWidget() override;

signals:
    void saveRequested();
    void cancelRequested();

private:
    ITrayLoadPlugin* m_plugin;
    QWidget* m_settingsWidget;
};

#endif // PLUGINSETTINGSWIDGET_H