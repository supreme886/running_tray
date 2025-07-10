#ifndef APPSETTINGSWIDGET_H
#define APPSETTINGSWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class QCheckBox;
class QPushButton;

class AppSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit AppSettingsWidget(QWidget *parent = nullptr);

signals:
    void saveRequested();

private:
    QVBoxLayout* settingsLayout;
    QCheckBox* autoStartCheckBox;
    QPushButton* saveBtn;
};

#endif // APPSETTINGSWIDGET_H