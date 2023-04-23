#include "requestmapper.h"

void RequestMapper::settings_reader(QSettings *settings)
{

    QString def =  "Driver={ODBC Driver 17 for SQL Server};"
                  "Server=lpc:localhost\\SQLEXPRESS;Database=TCDB_Check;"//              "Port=1433;"
                  "Uid=TC;Pwd=tc12452904;"
                  //QString def = "Driver={SQL Server};Server=10.1.7.11;Database=TCDB_Check;Port=1433;Uid=TC;Pwd=tc12452904";
                  "CharSet=UTF16;Trusted_Connection=no;SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_WRITE_ONLY;"
                  // "SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
                  "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC4;"   // ниже 4 косячит
                  "SQL_ATTR_CONNECTION_TIMEOUT=2;SQL_ATTR_LOGIN_TIMEOUT=2;";
    //settings->endGroup();
    settings->beginGroup( "db" );
    conn_str = settings->value( "connection_string", def ).toString();// + QHostInfo::localHostName() + settings->value( "connection_string2" ).toString();
    if ( !( settings->contains("connection_string")) )
        ;
    //        emit logger("RequestMapper:: В конфиг файле отсутствует строка подключения, использовано значение по умолчанию!!\n");
    settings->endGroup();
    //    emit logger("Строка подключения к БД = \n" + conn_str);



}

RequestMapper::RequestMapper(QSettings *settings, HttpSessionStore *sessionStore, QObject *parent): HttpRequestHandler(parent), sessionStore(sessionStore) {
    // empty
    settings_reader(settings);

    // Static file controller
    settings->beginGroup("files");
    staticFileController=new StaticFileController(settings, this);
    settings->endGroup();
    _db_query_processor = new db_query_processor(conn_str, sessionStore, this);
    loginController = new LoginController(conn_str, sessionStore, this);


}

void RequestMapper::service(HttpRequest &request, HttpResponse &response) {
    QByteArray path=request.getPath();
    QByteArray sessionId = sessionStore->getSessionId(request,response);
    QByteArray username=request.getParameter("username");
    //_db_query_processor->service(sessionId, request, response);
    if (path.startsWith("/s") )    // это что бы не логиниться для теста сканера
    {
        staticFileController->service(request,response);
        return;
    }
    if (path == "/login.html" || path.startsWith("/js") )
    {
        staticFileController->service(request,response);
        return;
    }
    if (sessionId.isEmpty())
    {
        sessionStore->getSession(request,response,true);  //  Создадим сессию
        response.redirect("/login.html");
        return;
    }
    if (path == "/login")                     // Примем пост с логином и хэшем
    {
        loginController->service(sessionId, request, response);
        return;
    }
    if ( !sessionStore->getSession(sessionId).contains("username")) // Вдруг кто то создал сессию без имени или пароля
    {
        response.redirect("/login.html");
        return;
    }
    if (path == "/check-barcode")                     // Проверим билет
    {
        QByteArray barcode=request.getParameter("barcode");

        _db_query_processor->service(sessionId, request, response);
        qDebug("check barcode = %s",barcode.constData());

        //response.redirect("/");
        return;
    }



    if ( !path.isEmpty() )               // Тут обработаем все что попросят статичное
    {
        staticFileController->service(request,response);
        return;
    }
    else {
        response.setStatus(404,"Not found");
        response.write("The URL is wrong, no such document.");
    }
}
