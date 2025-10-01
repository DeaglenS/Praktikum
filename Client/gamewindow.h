#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H
#include "client.h"
#include <QDialog>

namespace Ui {
class GameWindow;
}

class GameWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = nullptr, Client *client = nullptr, QString Login = nullptr, QString Password = nullptr);
    ~GameWindow();

private slots:
    void on_rock_Button_clicked();

    void on_scissors_Button_clicked();

    void on_paper_Button_clicked();

    void onMessageReceived(const QString &message);

  //    void closeEvent(QCloseEvent * event);

private:
    Ui::GameWindow *ui;
public:
    Client *client;
    QString Login;
    QString Password;
};

#endif // GAMEWINDOW_H
