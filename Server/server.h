#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QMap>
#include <QDebug>
#include <customsocket.h>
#include "databasemanager.h"
#include <QMutex>
#include <QAbstractSocket>
#include <QRunnable>
#include <QObject>

struct Game {
    QString player1;
    QString player2;
    QString choice1;
    QString choice2;
    QString password;
    QDateTime startTime;
};
class Server;


class MessageProcessor : public QObject, public QRunnable {
    Q_OBJECT

private:
    QString msg;
    QObject *sender;
    Server *server;

public:
    MessageProcessor(const QString &message, QObject *sender, Server *server)
        : msg(message), sender(sender), server(server) {}

    void run() override;
    void askToPlay1(CustomSocket *socketToPlay); // Declaration of askToPlay
    void startNewGame1();
    void determineWinner1(const QString &gameId);
    void determineWinner1(const Game game);
    void askToPlayWithPassword1(CustomSocket *socketToPlay, const QString &password);
    CustomSocket* getClientById1(const QString &clientId);
    void startNewPasswordProtectedGame1(CustomSocket *player1Socket, CustomSocket *player2Socket, const QString &password);

signals:
    void databaseOperation(QObject *sender, const QString &command, const QString &login, const QString &name, const QString &password);
    void databaseAddOperation(const QString &login);
    void JsonSendOperation(const CustomSocket *client, const QJsonObject &responseObj);
};

class Server : public QTcpServer {
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    void startServer();


private slots:
    void newConnection();
    void processMessage(const QString &message);
    void delete_session(CustomSocket *socketToDelete);
    void handleDatabaseAddOperation(const QString &login);
    void handleDatabaseOperation(QObject *sender, const QString &command, const QString &login, const QString &name, const QString &password);
    void handleJsonSendOperation(const CustomSocket *client, const QJsonObject &responseObj);
private:
    void startNewGame();
    void determineWinner(const QString &gameId);
    void askToPlay(CustomSocket *socketToPlay);
    //void addWin(const QString &login);
    void sendJsonResponse(const QString &clientId, const QJsonObject &responseObj);
    void sendJsonResponse(const CustomSocket *client, const QJsonObject &responseObj);

    CustomSocket* getClientById(const QString &clientId);
    DatabaseManager dbManager;
    void handleLogin(CustomSocket *client, const QString &login, const QString &password);
    void handleCreateUser(CustomSocket *client, const QString &name, const QString &login, const QString &password);
    QMap<QString, QString> clientLoginMap;
    mutable QMutex clientLoginMapMutex;



    void determineWinner(const Game game);

    QVector<CustomSocket*> clientsToPlay;
    QMap<QString, Game> games;
    QMap<QString, QString> clientToGameMap;
    int gameCounter = 0;
    QVector<CustomSocket*> clientsInPlay;
    QVector<CustomSocket*> clientsToPlayWithPass;
    void startNewPasswordProtectedGame(CustomSocket *player1Socket, CustomSocket *player2Socket, const QString &password);
    void askToPlayWithPassword(CustomSocket *socketToPlay, const QString &password);
    void func1();
    void startGameChecker();
    mutable QMutex gamesMutex;

    void handleJsonSend(const CustomSocket *client, const QJsonObject &responseObj);
    //void handleJsonSendOperation(const CustomSocket *client, const QJsonObject &responseObj);

    //void handleDatabaseOperation( QObject *sender, const QString &command, const QString &login, const QString &name, const QString &password);
    friend class MessageProcessor;

    void addWin(const QString &login);
};

#endif // SERVER_H
