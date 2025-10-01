#include "databasemanager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QThread>
#include <QString>

DatabaseManager::DatabaseManager(const QString &path)//Только путь. Если надо другое название то менять в bool DatabaseManager::createTable().
    : path(path) //this->path = path;
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

}

DatabaseManager::~DatabaseManager() {
    if (db.isOpen()) {
        db.close();
    }
}

void DatabaseManager::close() {
    if (db.isOpen()) {
        db.close();
    }
}

bool DatabaseManager::connect() {
    if (!db.open()) {
        qDebug() << "Cannot open database:" << db.lastError();
        return false;
    }
    return true;
}

bool DatabaseManager::createTable() {
    if (!db.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }
    QSqlQuery query;
    if (!query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='players';")) {
        qDebug() << "Failed to check if table exists:" << query.lastError();
        return false;
    }

    if (query.next()) {
        // Таблица уже существует
        qDebug() << "Table 'players' already exists.";
        return true;
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS players ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "name TEXT NOT NULL, "
                    "wins INTEGER NOT NULL, "
                    "login TEXT NOT NULL UNIQUE, "
                    "password TEXT NOT NULL)"))
    {
        qDebug() << "Failed to create table:" << query.lastError();
        return false;
    }
    return true;
}
/*
QSqlDatabase DatabaseManager::getDatabaseConnection() {
    QString connectionName = QString("conn_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    if (!QSqlDatabase::contains(connectionName)) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(path);
        if (!db.open()) {
            qDebug() << "Failed to open database in thread" << QThread::currentThreadId();
        }
    }

    return QSqlDatabase::database(connectionName);
}
*/

void DatabaseManager::printDatabase() {
    QSqlQuery query;

    if (!query.exec("SELECT * FROM players")) {
        qDebug() << "Failed to select from players:" << query.lastError();
        return;
    }
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        int wins = query.value(2).toInt();
        QString login = query.value(3).toString();
        QString password = query.value(4).toString();

        qDebug() << "ID:" << id
                 << "Name:" << name
                 << "Wins:" << wins
                 << "Login:" << login
                 << "Password:" << password;
    }
}

bool DatabaseManager::createUser(const QString &name, const QString &login, const QString &password) {
    if (!db.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM players WHERE login = ?");
    query.addBindValue(login);

    if (!query.exec()) {
        qDebug() << "Failed to check if user exists:" << query.lastError();
        return false;
    }

    if (query.next()) {
        qDebug() << "User already exists!";
        return false;
    }

    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashedPassword = hash.toHex();

    query.prepare("INSERT INTO players (name, wins, login, password) VALUES (?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(0); // Начальное количество побед
    query.addBindValue(login);
    query.addBindValue(hashedPassword);

    if (!query.exec()) {
        qDebug() << "Failed to insert new user:" << query.lastError();
        return false;
    }

    qDebug() << "User added!";
    return true;
}

bool DatabaseManager::loginUser(const QString &login, const QString &password, int &wins) {
    if (!db.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT password, wins FROM players WHERE login = ?");
    query.addBindValue(login);

    if (!query.exec()) {
        qDebug() << "Failed to retrieve password:" << query.lastError();
        return false;
    }

    if (!query.next()) {
        qDebug() << "User not found!";
        return false;
    }

    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashedPassword = hash.toHex();

    if (query.value(0).toString() == hashedPassword) {
        qDebug() << "Login successful!";
        wins = query.value(1).toInt();
        return true;
    } else {
        qDebug() << "Incorrect password!";
        return false;
    }
}


bool DatabaseManager::loginUser(const QString &login, const QString &password) {
    if (!db.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT password FROM players WHERE login = ?");
    query.addBindValue(login);

    if (!query.exec()) {
        qDebug() << "Failed to retrieve password:" << query.lastError();
        return false;
    }

    if (!query.next()) {
        qDebug() << "User not found!";
        return false;
    }

    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashedPassword = hash.toHex();

    if (query.value(0).toString() == hashedPassword) {
        qDebug() << "Login successful!";
        return true;
    } else {
        qDebug() << "Incorrect password!";
        return false;
    }
}

bool DatabaseManager::addWin(const QString &login) {
    if (!db.isOpen()) {
        qDebug() << "Database is not open.";
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE players SET wins = wins + 1 WHERE login = ?");
    query.addBindValue(login);

    if (!query.exec()) {
        qDebug() << "Failed to update wins:" << query.lastError();
        return false;
    }

    return true;
}

QString DatabaseManager::getDatabasePath() const {
    return path;
}
