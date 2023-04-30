#include "httpserver/httplistener.h"
#include "http_server.h"

http_server::http_server(QSettings &_settings):settings(_settings)
{}

void http_server::start()
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
    QObject::connect(this,   &http_server::send_to_widgets,         server, &SslWebSocketServer::send_to_widgets );
    QObject::connect(server, &SslWebSocketServer::widget_readyRead, this,    &http_server::widget_readyRead );
}
