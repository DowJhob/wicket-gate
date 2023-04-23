#ifndef LOGINCONTROLLER_H
#define LOGINCONTROLLER_H
#include <QTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QTextCodec>

#include "httpserver/httprequesthandler.h"
#include "httpserver/httpsessionstore.h"

using namespace stefanfrings;

class LoginController : public HttpRequestHandler {
    Q_OBJECT
public:
    HttpSessionStore *sessionStore;
    QString conn_str;

    QTextCodec *codec = QTextCodec::codecForName("CP866");
    LoginController(QString conn_str, HttpSessionStore *sessionStore, QObject* parent=0);
    void create_db_connect(QString conn_str);
    bool db_open();
    int check_pass( QString user, QByteArray pass);
    void service(QByteArray sessionId, HttpRequest& request, HttpResponse& response);
};

#endif // LOGINCONTROLLER_H
