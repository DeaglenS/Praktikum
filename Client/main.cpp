#include "loginwindow.h"

#include <QApplication>
//#include "client.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

   // Client client;
   // client.connectToServer("127.0.0.1", 50002);
   /* Client client;
    client.connectToServer("127.0.0.1", 50002);
    QJsonObject message;
    message["command"] = "LOGIN";
    message["login"] = "login";
    message["password"] = "password";

    client.sendMessage(message);*/
   LoginWindow w;
   w.show();
 return a.exec();


}

//не забывать удалить сокет

