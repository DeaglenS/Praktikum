#ifndef ACCOUNTWINDOW_H
#define ACCOUNTWINDOW_H

#include <QDialog>
#include "client.h"
#include "gamewindow.h"
namespace Ui {
class AccountWindow;
}

class AccountWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AccountWindow(QWidget *parent = nullptr, Client *client = nullptr, QString Login = nullptr, QString Password = nullptr, int wins = 0);
    ~AccountWindow();

private slots:
    void on_pushButton_clicked();
    void onMessageReceived(const QString &message);

    void on_privateGame_pushButton_clicked();

  //    void closeEvent(QCloseEvent * event);

private:
    Ui::AccountWindow *ui;
    Client *client;
    QString Login;
    QString Password;
};

#endif // ACCOUNTWINDOW_H
