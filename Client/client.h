#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>


class Client : public QObject {
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);
    bool connectToServer(const QString &host, quint16 port);
    void sendMessage(const QJsonObject &message);
    void closeConnection();
    bool state();
    ~Client();

signals:
    void messageReceived(const QString &message);

private slots:
    void onConnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);



private:
    QTcpSocket *socket;


};

#endif // TESTCLIENT_H
