#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QTcpSocket>
#include <QTextStream>

class ClientSocket : public QTcpSocket {
    Q_OBJECT

public:
    explicit ClientSocket(QObject *parent = nullptr);

private slots:
    void readData();

signals:
    void messageReceived(const QString &message);
};

#endif // CLIENTSOCKET_H
