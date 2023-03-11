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
    http_server(QSettings& _settings);
    void start();
signals:
    void logger( QString );
    void widget_readyRead( QString );
    void send_to_widgets(QString);
//    void widget_readyRead(QString);
};

#endif // HTTP_SERVER_H
