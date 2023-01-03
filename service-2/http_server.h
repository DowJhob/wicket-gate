#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <QObject>

#include "httpserver/httplistener.h"
#include "http/requestmapper.h"
#include "ssl_websocket_server.h"

using namespace stefanfrings;

class http_server : public QObject
{
    Q_OBJECT
public:
SslWebSocketServer *server;
    QSettings& settings;
    http_server(QSettings& _settings):settings(_settings)
    {}
    void start()
    {
        // Session store
        settings.beginGroup("sessions");
        HttpSessionStore* sessionStore = new HttpSessionStore(&settings,this);
        settings.endGroup();

        RequestMapper *reqm = new RequestMapper(&settings, sessionStore);

        // HTTP server
        settings.beginGroup("listener");
        new HttpListener(&settings, reqm);
        settings.endGroup();

        server = new SslWebSocketServer(&settings, sessionStore, this);
        QObject::connect(server, &SslWebSocketServer::logger,           this,    &http_server::logger );
        QObject::connect(this,    &http_server::send_to_widgets,         server, &SslWebSocketServer::send_to_widgets );
        QObject::connect(server, &SslWebSocketServer::widget_readyRead, this,    &http_server::widget_readyRead );
    }
signals:
    void logger( QString );
    void widget_readyRead( QString );
    void send_to_widgets(QString);
//    void widget_readyRead(QString);
};

#endif // HTTP_SERVER_H
