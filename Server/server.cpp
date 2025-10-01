#include "server.h"
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include "customsocket.h"
#include <QtConcurrent>
#include <QThreadPool>

Server::Server(QObject *parent) : QTcpServer(parent), dbManager("db.db") {
    qDebug() << "Server object created.";

    if (!QFile::exists(dbManager.getDatabasePath())) {
        qDebug() << "Database does not exist. Creating a new one.";
        if (dbManager.connect()) {
            if (dbManager.createTable()) {
                qDebug() << "New database created successfully.";
            } else {
                qDebug() << "Failed to create new database.";
            }
            dbManager.close();
        } else {
            qDebug() << "Failed to connect to the database.";
        }
    } else {
        qDebug() << "Database exists.";
    }

    connect(this, &QTcpServer::newConnection, this, &Server::newConnection);
    startGameChecker();
}

void Server::startServer(){
    if (!listen(QHostAddress::Any, 50002)) {
        qDebug() << "Server could not start!";
    } else {
        qDebug() << "Server started!";
    }
}

void Server::newConnection() {
    QTcpSocket *socket = nextPendingConnection();
    if (socket) {
        qDebug() << "New pending connection received.";
        CustomSocket *customSocket = new CustomSocket(this);
        customSocket->setSocketDescriptor(socket->socketDescriptor());
        connect(customSocket, &CustomSocket::messageReceived, this, &Server::processMessage);
        qDebug() << "New connection established with descriptor:" << customSocket->socketDescriptor();
    } else {
        qDebug() << "Failed to get the next pending connection.";
    }
}

void Server::processMessage(const QString &message) {
    MessageProcessor *task = new MessageProcessor(message, sender(), this);
    connect(task, &MessageProcessor::databaseOperation, this, &Server::handleDatabaseOperation);
    connect(task, &MessageProcessor::databaseAddOperation, this, &Server::handleDatabaseAddOperation);
    connect(task, &MessageProcessor::JsonSendOperation, this, &Server::handleJsonSendOperation);
    QThreadPool::globalInstance()->start(task);
}

void Server::handleDatabaseOperation( QObject *sender, const QString &command, const QString &login, const QString &name, const QString &password) {
    if (command == "CREATE_USER") {
        CustomSocket *client = qobject_cast<CustomSocket*>(sender);
        if (dbManager.connect()) {
            QJsonObject response;
            if (dbManager.createUser(name, login, password)) {
                response["status"] = "success";
                response["message"] = "User created successfully!";
                qDebug() << "User added!";
            } else {
                response["status"] = "error";
                response["message"] = "User creation failed!";
            }

            // Ensure sendJsonResponse is called
            sendJsonResponse(client, response);
            dbManager.close();
        } else {
            QJsonObject response;
            response["status"] = "error";
            response["message"] = "DB problem";
            sendJsonResponse(client, response);
        }
    }
    if(command == "LOGIN"){
        CustomSocket *client = qobject_cast<CustomSocket*>(sender);
        if (dbManager.connect()) {
            QJsonObject response;
            int wins;
            if (dbManager.loginUser(login, password, wins)) {
                QString clientId = QString::number(client->socketDescriptor());
                clientLoginMap[clientId] = login;
                response["status"] = "success";
                response["message"] = "Login successful!";
                response["wins"] = wins;
            } else {
                response["status"] = "error";
                response["message"] = "Login failed!";
            }
            dbManager.close();
            sendJsonResponse(client, response);
        }
        else{
            QJsonObject response;
            response["status"] = "error";
            response["message"] = "DB problem";
            sendJsonResponse(client, response);
        }
    }
    // Handle other database operations here
}

void Server::handleJsonSend(const CustomSocket *client, const QJsonObject &responseObj) {
    if (client && client->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(responseObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        QDataStream out(const_cast<CustomSocket*>(client));
        out.setVersion(QDataStream::Qt_5_7);
        out << QString::fromUtf8(jsonData);

        qDebug() << "Sending response to client with descriptor:" << client->socketDescriptor();
    } else {
        qDebug() << "Client socket is null or not connected.";
    }
    // Handle other database operations here
}

void Server::handleJsonSendOperation(const CustomSocket *client, const QJsonObject &responseObj) {
    if (client && client->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(responseObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        QDataStream out(const_cast<CustomSocket*>(client));
        out.setVersion(QDataStream::Qt_5_7);
        out << QString::fromUtf8(jsonData);

        qDebug() << "Sending response to client with descriptor:" << client->socketDescriptor();
    } else {
        qDebug() << "Client socket is null or not connected.";
    }

    // Handle other database operations here
}


void Server::handleCreateUser(CustomSocket *client, const QString &name, const QString &login, const QString &password) {
    if (dbManager.connect()) {
        QJsonObject response;
        if (dbManager.createUser(name, login, password)) {
            response["status"] = "success";
            response["message"] = "User created successfully!";
        } else {
            response["status"] = "error";
            response["message"] = "User creation failed!";
        }

        if (client) {
            sendJsonResponse(client, response);
        }
        dbManager.close();
    } else {
        QJsonObject response;
        response["status"] = "error";
        response["message"] = "DB problem";
        if (client) {
            sendJsonResponse(client, response);
        }
    }
}

void Server::handleDatabaseAddOperation(const QString &login) {
    if(dbManager.connect()){
        dbManager.addWin(login);
        dbManager.close();
    }
}

void Server::askToPlay(CustomSocket *socketToPlay) {
    bool clientExists = false;
    for (CustomSocket *client : clientsToPlay) {
        if (client->socketDescriptor() == socketToPlay->socketDescriptor()) {
            clientExists = true;
            break;
        }
    }

    if (!clientExists) {
        clientsToPlay.append(socketToPlay);
    }

    if (clientsToPlay.size() >= 2) {
        startNewGame();
    }
}

void Server::startNewGame() {
    if (clientsToPlay.size() < 2) {
        return;
    }
    QString gameId = QString::number(gameCounter++);
    Game game;

    CustomSocket *player1Socket = clientsToPlay.takeAt(clientsToPlay.size() - 2);
    CustomSocket *player2Socket = clientsToPlay.takeAt(clientsToPlay.size() - 1);

    game.player1 = QString::number(player1Socket->socketDescriptor());
    game.player2 = QString::number(player2Socket->socketDescriptor());

    games[gameId] = game;

    clientToGameMap[game.player1] = gameId;
    clientToGameMap[game.player2] = gameId;

    clientsInPlay.append(player1Socket);
    clientsInPlay.append(player2Socket);

    QJsonObject response;
    response["type"] = "game_status";
    response["state"] = "started";

    sendJsonResponse(player1Socket, response);
    sendJsonResponse(player2Socket, response);

    qDebug() << "New game started with ID:" << gameId;
}

void MessageProcessor::startNewGame1() {
    if (server->clientsToPlay.size() < 2) {
        return;
    }
    QString gameId = QString::number(server->gameCounter++);
    Game game;

    CustomSocket *player1Socket = server->clientsToPlay.takeAt(server->clientsToPlay.size() - 2);
    CustomSocket *player2Socket = server->clientsToPlay.takeAt(server->clientsToPlay.size() - 1);

    game.player1 = QString::number(player1Socket->socketDescriptor());
    game.player2 = QString::number(player2Socket->socketDescriptor());

    server->games[gameId] = game;

    server->clientToGameMap[game.player1] = gameId;
    server->clientToGameMap[game.player2] = gameId;

    server->clientsInPlay.append(player1Socket);
    server->clientsInPlay.append(player2Socket);

    QJsonObject response;
    response["type"] = "game_status";
    response["state"] = "started";

    emit JsonSendOperation(player1Socket, response);

    emit JsonSendOperation(player2Socket, response);
   // sendJsonResponse(player1Socket, response);
   // sendJsonResponse(player2Socket, response);

    qDebug() << "New game started with ID:" << gameId;
}
/*
void Server::determineWinner(const QString &gameId) {
    Game game = games[gameId];
    QString result;
    qDebug() << "New resulting started with ID:" << gameId;
    bool is_player1_win = (game.choice1 == "Rock" && game.choice2 == "Scissors") ||
                          (game.choice1 == "Paper" && game.choice2 == "Rock") ||
                          (game.choice1 == "Scissors" && game.choice2 == "Paper");

    if(game.choice1.isEmpty() && !game.choice2.isEmpty()){
        result = "Player 1 wins";
        if (clientLoginMap.contains(game.player1))
        {
            addWin(clientLoginMap[game.player1]);
        }
    }
    else if(!game.choice1.isEmpty() && game.choice2.isEmpty()){
        result = "Player 2 wins";
        if (clientLoginMap.contains(game.player2))
        {
            addWin(clientLoginMap[game.player2]);
        }
    }
    else if (game.choice1 == game.choice2) {
        result = "Draw";
    } else if (is_player1_win)
    {
        result = "Player 1 wins";
        addWin(clientLoginMap[game.player1]);
        if (clientLoginMap.contains(game.player1))
        {
            addWin(clientLoginMap[game.player1]);
        }
    } else {
        result = "Player 2 wins";
        if (clientLoginMap.contains(game.player2))
        {
            addWin(clientLoginMap[game.player2]);
        }
    }

    QJsonObject response;
    response["type"] = "game_status";

    foreach (CustomSocket *client, clientsInPlay) {
        QString clientId = QString::number(client->socketDescriptor());
        if (clientId == game.player1) {
            if (result == "Draw") {
                response["result"] = "draw";
            } else {
                response["result"] = (result == "Player 1 wins") ? "win" : "lose";
            }
            sendJsonResponse(client, response);
        } else if (clientId == game.player2) {
            if (result == "Draw") {
                response["result"] = "draw";
            } else {
                response["result"] = (result == "Player 2 wins") ? "win" : "lose";
            }
            sendJsonResponse(client, response);
        }
    }

    games.remove(gameId);
    clientToGameMap.remove(game.player1);
    clientToGameMap.remove(game.player2);

    for (int i = 0; i < clientsInPlay.size(); ++i) {
        CustomSocket *client = clientsInPlay.at(i);
        if (QString::number(client->socketDescriptor()) == game.player1 ||
            QString::number(client->socketDescriptor()) == game.player2) {
            clientsInPlay.removeAt(i);
            --i; // Adjust index after removal
        }
    }
}
*/

void Server::determineWinner(const Game game) {
    QString result;

    if (game.choice1.isEmpty() || game.choice2.isEmpty()) {
        result = game.choice1.isEmpty() ? "Player 2 wins" : "Player 1 wins";
    } else {
        bool is_player1_win = (game.choice1 == "Rock" && game.choice2 == "Scissors") ||
                              (game.choice1 == "Paper" && game.choice2 == "Rock") ||
                              (game.choice1 == "Scissors" && game.choice2 == "Paper");

        if (game.choice1 == game.choice2) {
            result = "Draw";
        } else if (is_player1_win) {
            result = "Player 1 wins";
            if (clientLoginMap.contains(game.player1)) {
                addWin(clientLoginMap[game.player1]);
            }
        } else {
            result = "Player 2 wins";
            if (clientLoginMap.contains(game.player2)) {
                addWin(clientLoginMap[game.player2]);
            }
        }
    }

    QJsonObject response;
    response["type"] = "game_status";

    foreach (CustomSocket *client, clientsInPlay) {
        QString clientId = QString::number(client->socketDescriptor());
        if (clientId == game.player1) {
            if (result == "Draw") {
                response["result"] = "draw";
            } else {
                response["result"] = (result == "Player 1 wins") ? "win" : "lose";
            }
            sendJsonResponse(client, response);
        } else if (clientId == game.player2) {
            if (result == "Draw") {
                response["result"] = "draw";
            } else {
                response["result"] = (result == "Player 2 wins") ? "win" : "lose";
            }
            sendJsonResponse(client, response);
        }
    }

    clientToGameMap.remove(game.player1);
    clientToGameMap.remove(game.player2);

    for (int i = 0; i < clientsInPlay.size(); ++i) {
        CustomSocket *client = clientsInPlay.at(i);
        if (QString::number(client->socketDescriptor()) == game.player1 ||
            QString::number(client->socketDescriptor()) == game.player2) {
            clientsInPlay.removeAt(i);
            --i; // Adjust index after removal
        }
    }
}


void MessageProcessor::determineWinner1(const Game game) {
    QString result;

    if (game.choice1.isEmpty() || game.choice2.isEmpty()) {
        result = game.choice1.isEmpty() ? "Player 2 wins" : "Player 1 wins";
    } else {
        bool is_player1_win = (game.choice1 == "Rock" && game.choice2 == "Scissors") ||
                              (game.choice1 == "Paper" && game.choice2 == "Rock") ||
                              (game.choice1 == "Scissors" && game.choice2 == "Paper");

        if (game.choice1 == game.choice2) {
            result = "Draw";
        } else if (is_player1_win) {
            result = "Player 1 wins";
            if (server->clientLoginMap.contains(game.player1)) {
                emit databaseAddOperation(server->clientLoginMap[game.player1]);//addWin(server->clientLoginMap[game.player1]);
            }
        } else {
            result = "Player 2 wins";
            if (server->clientLoginMap.contains(game.player2)) {
                  emit databaseAddOperation(server->clientLoginMap[game.player2]);
            }
        }
    }

    QJsonObject response;
    response["type"] = "game_status";

    foreach (CustomSocket *client, server->clientsInPlay) {
        QString clientId = QString::number(client->socketDescriptor());
        if (clientId == game.player1) {
            if (result == "Draw") {
                response["result"] = "draw";
            } else {
                response["result"] = (result == "Player 1 wins") ? "win" : "lose";
            }
            emit JsonSendOperation(client, response);
        } else if (clientId == game.player2) {
            if (result == "Draw") {
                response["result"] = "draw";
            } else {
                response["result"] = (result == "Player 2 wins") ? "win" : "lose";
            }
            emit JsonSendOperation(client, response);
        }
    }

    server->clientToGameMap.remove(game.player1);
    server->clientToGameMap.remove(game.player2);

    for (int i = 0; i < server->clientsInPlay.size(); ++i) {
        CustomSocket *client = server->clientsInPlay.at(i);
        if (QString::number(client->socketDescriptor()) == game.player1 ||
            QString::number(client->socketDescriptor()) == game.player2) {
            server->clientsInPlay.removeAt(i);
            --i; // Adjust index after removal
        }
    }
}


void MessageProcessor::determineWinner1(const QString &gameId) {
    Game game = server->games[gameId];
    QString result;
    qDebug() << "New resulting started with ID:" << gameId;
    bool is_player1_win = (game.choice1 == "Rock" && game.choice2 == "Scissors") ||
                          (game.choice1 == "Paper" && game.choice2 == "Rock") ||
                          (game.choice1 == "Scissors" && game.choice2 == "Paper");

    if(game.choice1.isEmpty() && !game.choice2.isEmpty()){
        result = "Player 1 wins";
        if (server->clientLoginMap.contains(game.player1))
        {
            emit databaseAddOperation(server->clientLoginMap[game.player1]);
        }
    }
    else if(!game.choice1.isEmpty() && game.choice2.isEmpty()){
        result = "Player 2 wins";
        if (server->clientLoginMap.contains(game.player2))
        {
             emit databaseAddOperation(server->clientLoginMap[game.player2]);
        }
    }
    else if (game.choice1 == game.choice2) {
        result = "Draw";
    } else if (is_player1_win)
    {
        result = "Player 1 wins";
        //addWin(clientLoginMap[game.player1]);
        if (server->clientLoginMap.contains(game.player1))
        {
            emit databaseAddOperation(server->clientLoginMap[game.player1]);
        }
    } else {
        result = "Player 2 wins";
        if (server->clientLoginMap.contains(game.player2))
        {
             emit databaseAddOperation(server->clientLoginMap[game.player2]);
        }
    }

    QJsonObject response;
    response["type"] = "game_status";

    foreach (CustomSocket *client, server->clientsInPlay) {
        QString clientId = QString::number(client->socketDescriptor());
        if (clientId == game.player1) {
            if (result == "Draw") {
                response["result"] = "draw";
            } else {
                response["result"] = (result == "Player 1 wins") ? "win" : "lose";
            }
           emit JsonSendOperation(client, response);
        } else if (clientId == game.player2) {
            if (result == "Draw") {
                response["result"] = "draw";
            } else {
                response["result"] = (result == "Player 2 wins") ? "win" : "lose";
            }
            emit JsonSendOperation(client, response);
        }
    }

    server->games.remove(gameId);
    server->clientToGameMap.remove(game.player1);
    server->clientToGameMap.remove(game.player2);

    for (int i = 0; i < server->clientsInPlay.size(); ++i) {
        CustomSocket *client = server->clientsInPlay.at(i);
        if (QString::number(client->socketDescriptor()) == game.player1 ||
            QString::number(client->socketDescriptor()) == game.player2) {
            server->clientsInPlay.removeAt(i);
            --i; // Adjust index after removal
        }
    }
}
CustomSocket* Server::getClientById(const QString &clientId) {
    foreach (CustomSocket *client, clientsToPlay) {
        if (QString::number(client->socketDescriptor()) == clientId) {
            return client;
        }
    }
    foreach (CustomSocket *client, clientsInPlay) {
        if (QString::number(client->socketDescriptor()) == clientId) {
            return client;
        }
    }
    return nullptr;
}

CustomSocket* MessageProcessor::getClientById1(const QString &clientId) {
    foreach (CustomSocket *client, server->clientsToPlay) {
        if (QString::number(client->socketDescriptor()) == clientId) {
            return client;
        }
    }
    foreach (CustomSocket *client, server->clientsInPlay) {
        if (QString::number(client->socketDescriptor()) == clientId) {
            return client;
        }
    }
    return nullptr;
}


void Server::sendJsonResponse(const QString &clientId, const QJsonObject &responseObj) {
    CustomSocket *client = getClientById(clientId);
    if (client) {
        QJsonDocument doc(responseObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        QDataStream out(client);
        out.setVersion(QDataStream::Qt_5_7);
        out << QString::fromUtf8(jsonData);

        qDebug() << "Sending response to client with descriptor:" << client->socketDescriptor();
    } else {
        qDebug() << "Client not found for ID:" << clientId;
    }
}

void Server::sendJsonResponse(const CustomSocket *client, const QJsonObject &responseObj) {
    if (client && client->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(responseObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        QDataStream out(const_cast<CustomSocket*>(client));
        out.setVersion(QDataStream::Qt_5_7);
        out << QString::fromUtf8(jsonData);

        qDebug() << "Sending response to client with descriptor:" << client->socketDescriptor();
    } else {
        qDebug() << "Client socket is null or not connected.";
    }
}

void Server::delete_session(CustomSocket *socketToDelete) {
    QString clientId = QString::number(socketToDelete->socketDescriptor());

    if (clientsToPlay.removeOne(socketToDelete)) {
        qDebug() << "Client socket successfully removed from clientsToPlay.";
    } else {
        qDebug() << "Client socket not found in clientsToPlay.";
    }

    if (clientsInPlay.removeOne(socketToDelete)) {
        qDebug() << "Client socket successfully removed from clientsInPlay.";
    } else {
        qDebug() << "Client socket not found in clientsInPlay.";
    }
    if (clientsToPlayWithPass.removeOne(socketToDelete)) {
        qDebug() << "Client socket successfully removed from clientsInPlay.";
    } else {
        qDebug() << "Client socket not found in clientsInPlay.";
    }

    socketToDelete->close();
    socketToDelete->deleteLater();
}

void Server::askToPlayWithPassword(CustomSocket *socketToPlay, const QString &password) {
    bool clientExists = false;
    for (CustomSocket *client : clientsToPlayWithPass) {
        if (client->socketDescriptor() == socketToPlay->socketDescriptor()) {
            clientExists = true;
            break;
        }
    }

    if (!clientExists) {
        clientsToPlayWithPass.append(socketToPlay);
    }

    QString existingGameId;
    for (const QString &gameId : games.keys()) {
        if (games[gameId].password == password && games[gameId].player2.isEmpty()) {
            existingGameId = gameId;
            break;
        }
    }

    if (!existingGameId.isEmpty()) {
        Game &game = games[existingGameId];
        game.player2 = QString::number(socketToPlay->socketDescriptor());
        clientToGameMap[game.player2] = existingGameId;

        clientsInPlay.append(socketToPlay);
        for (CustomSocket *client : clientsToPlayWithPass) {
            if (QString::number(client->socketDescriptor()) == game.player1) {
                clientsInPlay.append(client);
                break;
            }
        }

        clientsToPlayWithPass.removeOne(socketToPlay);
        for (CustomSocket *client : clientsToPlayWithPass) {
            if (QString::number(client->socketDescriptor()) == game.player1) {
                clientsToPlayWithPass.removeOne(client);
                break;
            }
        }

        QJsonObject response;
        response["type"] = "game_status";
        response["state"] = "started";
        sendJsonResponse(socketToPlay, response);
        sendJsonResponse(getClientById(game.player1), response);

        qDebug() << "Player joined existing password-protected game with ID:" << existingGameId;
    } else {
        startNewPasswordProtectedGame(socketToPlay, nullptr, password);
    }
}

void MessageProcessor::askToPlayWithPassword1(CustomSocket *socketToPlay, const QString &password) {
    bool clientExists = false;
    for (CustomSocket *client : server->clientsToPlayWithPass) {
        if (client->socketDescriptor() == socketToPlay->socketDescriptor()) {
            clientExists = true;
            break;
        }
    }

    if (!clientExists) {
        server->clientsToPlayWithPass.append(socketToPlay);
    }

    QString existingGameId;
    for (const QString &gameId : server->games.keys()) {
        if (server->games[gameId].password == password && server->games[gameId].player2.isEmpty()) {
            existingGameId = gameId;
            break;
        }
    }

    if (!existingGameId.isEmpty()) {
        Game &game = server->games[existingGameId];
        game.player2 = QString::number(socketToPlay->socketDescriptor());
        server->clientToGameMap[game.player2] = existingGameId;

        server->clientsInPlay.append(socketToPlay);
        for (CustomSocket *client : server->clientsToPlayWithPass) {
            if (QString::number(client->socketDescriptor()) == game.player1) {
                server->clientsInPlay.append(client);
                break;
            }
        }

        server->clientsToPlayWithPass.removeOne(socketToPlay);
        for (CustomSocket *client : server->clientsToPlayWithPass) {
            if (QString::number(client->socketDescriptor()) == game.player1) {
                server->clientsToPlayWithPass.removeOne(client);
                break;
            }
        }

        QJsonObject response;
        response["type"] = "game_status";
        response["state"] = "started";
       //sendJsonResponse(socketToPlay, response);
        emit JsonSendOperation(socketToPlay, response);
        emit JsonSendOperation(getClientById1(game.player1), response);
       // sendJsonResponse(server->getClientById(game.player1), response);

        qDebug() << "Player joined existing password-protected game with ID:" << existingGameId;
    } else {
       startNewPasswordProtectedGame1(socketToPlay, nullptr, password);
    }
}
void Server::startNewPasswordProtectedGame(CustomSocket *player1Socket, CustomSocket *player2Socket, const QString &password) {
    QString gameId = QString::number(gameCounter++);
    Game game;
    game.password = password;
    game.player1 = QString::number(player1Socket->socketDescriptor());
    if (player2Socket) {
        game.player2 = QString::number(player2Socket->socketDescriptor());
    }

    games[gameId] = game;

    clientToGameMap[game.player1] = gameId;
    if (!game.player2.isEmpty()) {
        clientToGameMap[game.player2] = gameId;
    }

    clientsInPlay.append(player1Socket);
    if (player2Socket) {
        clientsInPlay.append(player2Socket);
    }

    clientsToPlayWithPass.removeOne(player1Socket);
    if (player2Socket) {
        clientsToPlayWithPass.removeOne(player2Socket);
    }

    qDebug() << "New password-protected game started with ID:" << gameId;
}
void MessageProcessor::startNewPasswordProtectedGame1(CustomSocket *player1Socket, CustomSocket *player2Socket, const QString &password) {
    QString gameId = QString::number(server->gameCounter++);
    Game game;
    game.password = password;
    game.player1 = QString::number(player1Socket->socketDescriptor());
    if (player2Socket) {
        game.player2 = QString::number(player2Socket->socketDescriptor());
    }

    server->games[gameId] = game;

    server->clientToGameMap[game.player1] = gameId;
    if (!game.player2.isEmpty()) {
        server->clientToGameMap[game.player2] = gameId;
    }

    server->clientsInPlay.append(player1Socket);
    if (player2Socket) {
        server->clientsInPlay.append(player2Socket);
    }

    server->clientsToPlayWithPass.removeOne(player1Socket);
    if (player2Socket) {
        server->clientsToPlayWithPass.removeOne(player2Socket);
    }

    qDebug() << "New password-protected game started with ID:" << gameId;
}
void Server::startGameChecker() {
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Server::func1);
    timer->start(1000);
}

void Server::func1() {
    QMutexLocker locker(&gamesMutex);
    QDateTime currentTime = QDateTime::currentDateTime();

    for (auto it = games.begin(); it != games.end(); ++it) {
        const Game& game = it.value();
        if (game.startTime.secsTo(currentTime) >= 10) {
            determineWinner(game);
        }
    }
}
void MessageProcessor::askToPlay1(CustomSocket *socketToPlay) {
    bool clientExists = false;
    for (CustomSocket *client : server->clientsToPlay) {
        if (client->socketDescriptor() == socketToPlay->socketDescriptor()) {
            clientExists = true;
            break;
        }
    }

    if (!clientExists) {
        server->clientsToPlay.append(socketToPlay);
    }

    if (server->clientsToPlay.size() >= 2) {
        startNewGame1();
    }
}
void MessageProcessor::run() {

    /////
    ///
    ///
    ///
    ///
    ///
    ///


    /////
    qDebug() << "Обрабатываем сообщение в потоке:" << QThread::currentThread();
    qDebug() << "Server received request:" << msg;

    CustomSocket *senderSocket = qobject_cast<CustomSocket*>(sender);
    if (senderSocket) {
        QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
        if (!doc.isObject()) {
            qDebug() << "1JSON problem!";
        }

        QJsonObject json = doc.object();

        if (json.contains("command")) {
            QString command = json["command"].toString();

            if (command == "LOGIN") {
                // Handle login in the main thread
                // emit databaseOperation("LOGIN", json["login"].toString(), "", json["password"].toString());
                emit databaseOperation(sender, "LOGIN", json["login"].toString(), "", json["password"].toString());
            }
            else if (command == "CREATE_USER") {
                // Handle create user in the main thread
              //  handleDatabaseOperation1(sender, "CREATE_USER", json["login"].toString(), json["name"].toString(), json["password"].toString());
                emit databaseOperation(sender, "CREATE_USER", json["login"].toString(), json["name"].toString(), json["password"].toString());
                    /*
                    CustomSocket *client = qobject_cast<CustomSocket*>(sender);
                    QJsonObject response;
                    response["status"] = "success";
                    emit JsonSendOperation(client, response);
                    */

            }
            else if (command == "CREATE_GAME") {
                if (json.contains("password")) {
                    QString password = json["password"].toString();
                    //server->askToPlayWithPassword(senderSocket, password);
                    askToPlayWithPassword1(senderSocket, password);
                } else {
                    //server->askToPlay(senderSocket);
                      askToPlay1(senderSocket);
                }
            }else if(command == "START_ANONYM_GAME")
            {
                askToPlay1(senderSocket);
               // server->askToPlay(senderSocket);

            }
            else if(command == "DELETE_SESSION") {
                server->delete_session(senderSocket);
            }
            else if(command == "GET_INFO") {
                QJsonObject response;
                QString clientId = QString::number(senderSocket->socketDescriptor());
                response["socketDescriptor"] = clientId;
                emit JsonSendOperation (senderSocket, response);
                //server->sendJsonResponse(senderSocket, response);
            }
            else if (command == "MAKE_CHOOSE") {
                QString gameId = server->clientToGameMap[QString::number(senderSocket->socketDescriptor())];
                if (server->games[gameId].player1 == QString::number(senderSocket->socketDescriptor())) {
                    server->games[gameId].choice1 = json["choose"].toString();
                } else {
                    server->games[gameId].choice2 = json["choose"].toString();
                }

                if (!server->games[gameId].choice1.isEmpty() && !server->games[gameId].choice2.isEmpty()) {
                    determineWinner1(gameId);
                }
                else{
                    server->games[gameId].startTime = QDateTime::currentDateTime();
                }
            }
            else{
                qDebug() << "Unknown command.";
            }
        }
        else{
            QJsonObject response;
            response["status"] = "error";
            response["message"] = "Command does not exist!";

            server->sendJsonResponse(qobject_cast<CustomSocket*>(sender), response);
        }
    }

    qDebug() << "Сообщение обработано:" << msg;
}
void Server::addWin(const QString &login) {
    if(dbManager.connect()){
        dbManager.addWin(login);
        dbManager.close();
    }

}
