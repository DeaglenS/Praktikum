#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>

class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(QTcpSocket *socket, QObject *parent = nullptr);

public slots:
    void processMessage();
    void handleDisconnect();

private:
    QTcpSocket *socket;

    void handleCommand(const QJsonObject &json);
};

#endif // WORKER_H
