#ifndef REGISTRATIONWINDOW_H
#define REGISTRATIONWINDOW_H
#include "client.h"
#include <QDialog>
#include <QJsonObject>

namespace Ui {
class RegistrationWindow;
}

class RegistrationWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RegistrationWindow(QWidget *parent = nullptr, Client *client = nullptr);
    ~RegistrationWindow();

private slots:
    void on_makeAcc_Button_clicked();

    void on_back_Button_clicked();
    void onMessageReceived(const QString &message);

    void closeEvent(QCloseEvent * event);




private:
    Ui::RegistrationWindow *ui;
    Client *client;
};

#endif // REGISTRATIONWINDOW_H
