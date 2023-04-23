#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "httpserver/httprequesthandler.h"
#include "httpserver/httpsessionstore.h"
#include "http/staticfilecontroller.h"

#include "http/db-query-processor.h"
#include "http/logincontroller.h"
//#include "cookietestcontroller.h"


using namespace stefanfrings;

class RequestMapper : public HttpRequestHandler {
    Q_OBJECT
public:
    StaticFileController* staticFileController;
    HttpSessionStore* sessionStore;
    QString conn_str;

    void settings_reader(QSettings* settings = nullptr);

    RequestMapper(QSettings *settings, HttpSessionStore* sessionStore, QObject* parent=0);
    void service( HttpRequest& request, HttpResponse& response);
private:

    LoginController *loginController;
    db_query_processor *_db_query_processor;
    //    CookieTestController cookieTestController;

};

#endif // REQUESTMAPPER_H
