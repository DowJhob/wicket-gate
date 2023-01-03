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
    LoginController(QString conn_str, HttpSessionStore *sessionStore, QObject* parent=0) :
        HttpRequestHandler(parent), sessionStore(sessionStore)//, conn_str(conn_str)
    {
        // empty
        this->conn_str=
                "Driver={ODBC Driver 17 for SQL Server};"
        "Server=10.1.7.11;"
        "Database=TCDB;Uid=TC;Pwd=tc12452904;"
        "CharSet=UTF16;Trusted_Connection=no;"
        "SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_WRITE_ONLY;SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC4;SQL_ATTR_CONNECTION_TIMEOUT=2;SQL_ATTR_LOGIN_TIMEOUT=2;";

    }
    void create_db_connect(QString conn_str)
    {
        QSqlDatabase db = QSqlDatabase::addDatabase( "QODBC3", "htpp" );
        db.setDatabaseName( conn_str );
        db_open();
    }
    bool db_open()
    {
        QSqlDatabase db = QSqlDatabase::database("htpp", true);
        if ( !db.isValid() )
            return false;
        if ( !db.isOpen() )
        {
            //            emit logger(GateCaption + "  db worker: Соединение с БД закрыто,  пробуем открыть :");
            if ( !db.open() )
            {
                qDebug() <<"open : "<< QString::fromLocal8Bit( codec->fromUnicode( db.lastError().text() ) );
                //                emit logger(GateCaption + "  db worker: Не удалось соединиться с базой данных ! :");
                return false;
            }
            //qDebug() << db.lastError();
        }
        //        emit logger("database open succefull");
        return true;
    }
    int check_pass( QString user, QByteArray pass)
    {
        create_db_connect( conn_str);
        if ( !db_open() )                   //пробуем открыть
            return 0;
        QSqlQuery CheckUserIdentity_query( QSqlDatabase::database( "htpp" ) );
        CheckUserIdentity_query.setForwardOnly(true);
        CheckUserIdentity_query.prepare("{ call dbo.usp__CheckUserIdentity(:userLogin, :passwordHash) }");
        int role = 0;
        int flag = 0;
        CheckUserIdentity_query.bindValue(":userLogin", user, QSql::In );
        CheckUserIdentity_query.bindValue(":passwordHash", pass, QSql::In);
        if (!CheckUserIdentity_query.exec())
        {
            qDebug() << "CheckUserIdentity_query last error: " +
                        QString::fromLocal8Bit( codec->fromUnicode( CheckUserIdentity_query.lastError().text() ) );
            //     emit logger("check_ticket QUERY last error: " + check_ticket_query.lastError().text() );
        }
        while ( CheckUserIdentity_query.next() )
        {
            //     MAC = CheckUserIdentity_query.value(0).toString();
            //      gate.name = CheckUserIdentity_query.value(1).toString();
            //     gate.num = CheckUserIdentity_query.value(2).toInt();
            role = CheckUserIdentity_query.value(3).toInt();
            flag = CheckUserIdentity_query.value(4).toInt();
            qDebug() << "role: " << role;
            qDebug() << "flag: " << flag;
        }
        CheckUserIdentity_query.clear();
        CheckUserIdentity_query.finish();
        return flag;
    }
    void service(QByteArray sessionId, HttpRequest& request, HttpResponse& response) {
        QByteArray username=request.getParameter("username");
        QByteArray password=request.getParameter("password");
        qDebug("username=%s",username.constData());
        qDebug("password=%s",password.constData());
        //        response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");
        HttpSession session = sessionStore->getSession(sessionId);

        if (session.contains("username")) {
            //QByteArray username=session.get("username").toByteArray();
            // QTime logintime=session.get("logintime").toTime();

            // response.write("<html><body>You are alread logged in.<br>Your name is: "+username+"<br>"
            // "You logged in at: "+logintime.toString("HH:mm:ss").toLocal8Bit()+"<br>"
            // "</body></html>",true);
            response.redirect("/");
        }
        else
        {
            if (check_pass(username, QByteArray::fromHex(password)) > 0) {
                //  response.write("Yes, correct");
                session.set("username",username);
                session.set("logintime",QTime::currentTime());
                response.redirect("/");
                qDebug()<<"check_pass ";
            }
            else {
                qDebug()<<"wrong";
                sessionStore->removeSession(session);
                response.write("<html><body>"
                               "No, that was wrong!"
                               "</body></html>",true);
            }
        }
    }
};

#endif // LOGINCONTROLLER_H
