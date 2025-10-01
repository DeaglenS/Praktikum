#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H
#include "client.h"
#include <QMainWindow>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginWindow;
}
QT_END_NAMESPACE

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void on_auth_Button_clicked();
    void onMessageReceived(const QString &message);
    void on_registration_Button_clicked();
   //   void closeEvent(QCloseEvent * event);

    void on_guest_Button_clicked();

private:
    Ui::LoginWindow *ui;
    Client *client;
    QString login;
    QString password;

protected:



};
#endif // LOGINWINDOW_H
