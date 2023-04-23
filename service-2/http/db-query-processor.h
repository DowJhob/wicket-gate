#ifndef DB_QUERY_PROCESSOR_H
#define DB_QUERY_PROCESSOR_H
#include <QTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QTextCodec>

#include "httpserver/httprequesthandler.h"
#include "httpserver/httpsessionstore.h"

using namespace stefanfrings;

class db_query_processor : public HttpRequestHandler {
    Q_OBJECT
public:
    HttpSessionStore *sessionStore;
    QString conn_str;

    QTextCodec *codec = QTextCodec::codecForName ("CP866");
    db_query_processor(QString conn_str, HttpSessionStore *sessionStore, QObject* parent=0);
    void create_db_connect(QString conn_str);
    bool db_open();
    QString check_ticket( QString barcode);
    void service(QByteArray sessionId, HttpRequest& request, HttpResponse& response);
};

#endif // DB_QUERY_PROCESSOR_H
