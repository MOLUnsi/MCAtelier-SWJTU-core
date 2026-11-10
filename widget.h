#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStandardPaths>
#include <QThread>
#include <QDebug>
#include <QFileDialog>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>

#include "ui_widget.h"
#include "FileFunctions.h"
#include "CreateLaunchScript.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

private slots:
    void on_comboBox_activated(int index);

    void on_pushButton_clicked();

    void on_MinecraftPathLineEdit_editingFinished();

    void on_ChooseMinecraftPathFolder_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
