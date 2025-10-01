#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QCryptographicHash>
#include <QString>

class DatabaseManager {
public:
    DatabaseManager(const QString &path);
    ~DatabaseManager();

    bool connect();
    void close();
    bool createTable();
    void printDatabase();
    bool createUser(const QString &name, const QString &login, const QString &password);
    bool loginUser(const QString &login, const QString &password, int &wins);
    bool loginUser(const QString &login, const QString &password);
    bool addWin(const QString &login);
    QSqlDatabase getDatabaseConnection();
    QString getDatabasePath() const;
private:
    QSqlDatabase db;
    const QString &path;

};

#endif // DATABASEMANAGER_H
