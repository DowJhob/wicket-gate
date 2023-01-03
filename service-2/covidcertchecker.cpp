#include "covidcertchecker.h"

covidCertChecker::covidCertChecker(QObject *parent) : QObject(parent)
{
    //QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    //config.setProtocol(QSsl::TlsV1_0OrLater);
    //request.setSslConfiguration(config);

    //request.setHeader(QNetworkRequest::ServerHeader, "application/json");

    //QUrlQuery query;
    //query.addQueryItem("lang", "55");
    //query.addQueryItem("ck", "12");
    //CovidAPIURL.setQuery(query);
    QObject::connect(&manager, &QNetworkAccessManager::finished, this, &covidCertChecker::managerFinished);

}

void covidCertChecker::getCertCheck(QUrl certUrl)
{
    if(certUrl.host().contains(gosuslugiHost) //&& certUrl.path().contains("covid-cert")
            )
    {
        QString uid_unrz = certUrl.path().split('/').constLast();
        //qDebug() << "uid_unrz" << uid_unrz;
        QString query = certUrl.query();
        if(certUrl.path().contains("verify"))
        {
            certUrl.setPath(CovidAPIURL + uid_unrz);
        }
        else
            if(certUrl.path().contains("status"))
            {
                certUrl.setPath(newCovidAPIURL + uid_unrz);
            }
            else
            {
                emit certFail("Не распознан путь сертификата");
                return;
            }

        query.replace("lang=en", "lang=ru", Qt::CaseInsensitive);
        certUrl.setQuery(query);
        qDebug() << "\n request api url" << certUrl.toDisplayString() << "\n";
        request.setUrl(certUrl);
        manager.get(request);
    }
    else
        emit certFail("Неверный домен");
}

void covidCertChecker::managerFinished(QNetworkReply *reply)
{
    if (reply->error())
    {
        qDebug() << reply->errorString();
        emit certFail(reply->errorString());
        //return;
    }
    else
    {
        QByteArray answer = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(answer);
        //if (doc.is)
        certCheck(doc);
    }
    reply->deleteLater();
}

void covidCertChecker::certCheck(QJsonDocument doc)
{
    for (auto o : doc["items"].toArray())
    {
        auto oo = o.toObject();
        if(QDate::currentDate() < QDate::fromString(oo["expiredAt"].toString(), "dd.MM.yyyy"))
        {
            emit certOK(oo["attrs"].toArray());
            return;
        }
    }
    emit certFail("Не обнаружены действующие сертификаты");
}
