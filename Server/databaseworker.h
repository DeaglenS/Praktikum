#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QObject>
#include <QSqlDatabase>
#include <QJsonObject>

class DatabaseWorker : public QObject {
    Q_OBJECT

public:
    explicit DatabaseWorker(QObject *parent = nullptr);
    ~DatabaseWorker();

public slots:
    void handleDatabaseOperation(const QString &command, const QString &login, const QString &name, const QString &password);

signals:
    void databaseOperationResult(const QJsonObject &response);

private:
    QSqlDatabase db;
    bool connectToDatabase();
    void createUser(const QString &name, const QString &login, const QString &password, QJsonObject &response);
};

#endif // DATABASEWORKER_H
