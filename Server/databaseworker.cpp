#include "DatabaseWorker.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseWorker::DatabaseWorker(QObject *parent) : QObject(parent) {
    db = QSqlDatabase::addDatabase("QSQLITE", "worker_connection");
}

DatabaseWorker::~DatabaseWorker() {
    if (db.isOpen()) {
        db.close();
    }
    QSqlDatabase::removeDatabase("worker_connection");
}

bool DatabaseWorker::connectToDatabase() {
    db.setDatabaseName("db.db");
    if (!db.open()) {
        qDebug() << "Failed to connect to the database:" << db.lastError().text();
        return false;
    }
    return true;
}

void DatabaseWorker::handleDatabaseOperation(const QString &command, const QString &login, const QString &name, const QString &password) {
    QJsonObject response;
    if (command == "CREATE_USER") {
        createUser(name, login, password, response);
    }
    emit databaseOperationResult(response);
}

void DatabaseWorker::createUser(const QString &name, const QString &login, const QString &password, QJsonObject &response) {
    if (connectToDatabase()) {
        QSqlQuery query(db);
        query.prepare("INSERT INTO users (name, login, password) VALUES (:name, :login, :password)");
        query.bindValue(":name", name);
        query.bindValue(":login", login);
        query.bindValue(":password", password);

        if (query.exec()) {
            response["status"] = "success";
            response["message"] = "User created successfully!";
        } else {
            response["status"] = "error";
            response["message"] = "User creation failed!";
            qDebug() << "Failed to create user:" << query.lastError().text();
        }
        db.close();
    } else {
        response["status"] = "error";
        response["message"] = "DB problem";
    }
}
