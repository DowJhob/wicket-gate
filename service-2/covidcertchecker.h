#ifndef COVIDCERTCHECKER_H
#define COVIDCERTCHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

class covidCertChecker : public QObject
{
    Q_OBJECT
public:
    QString gosuslugiHost = "gosuslugi.ru";
    explicit covidCertChecker(QObject *parent = nullptr);

public slots:
    void getCertCheck(QUrl certUrl);

private:
    QNetworkAccessManager manager;
    QNetworkRequest request;
    QString CovidAPIURL = "/api/covid-cert/v3/cert/check/";  // с унрз
    QString newCovidAPIURL = "/api/covid-cert/v2/cert/status/";   //с уидом

    void certCheck(QJsonDocument doc);

private slots:
    void managerFinished(QNetworkReply *reply);

signals:
    void certOK(QJsonArray);
    void certFail(QString);

};

#endif // COVIDCERTCHECKER_H
