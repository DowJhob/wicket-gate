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
    db_query_processor(QString conn_str, HttpSessionStore *sessionStore, QObject* parent=0) :
        HttpRequestHandler(parent), sessionStore(sessionStore)//, conn_str(conn_str)
    {
        // empty
        this->conn_str=
                "Driver={ODBC Driver 17 for SQL Server};"
        "Server=10.1.7.11;"
        "Database=TCDB_Check;Uid=TC;Pwd=tc12452904;"
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
    QString check_ticket( QString barcode)
    {
        create_db_connect( conn_str);
        if ( !db_open() )                   //пробуем открыть
            return 0;
        QString response = "<table class =""td-black""><tr>"
                    "<th style =""min-width:8%"">Название группы</th>"
                    "<th style =""min-width:2%"">Номер</th>"
                    "<th style =""min-width:40%"">Действие</th>"
                    "<th style =""min-width:20%"">Время</th></tr>";

        QSqlQuery CheckTicketFromWidget_query( QSqlDatabase::database( "htpp" ) );
        CheckTicketFromWidget_query.setForwardOnly(true);
        CheckTicketFromWidget_query.prepare("{ call dbo.usp__CheckTicketFromWidget(:barcode) }");
        CheckTicketFromWidget_query.bindValue(":barcode", barcode, QSql::In );
        if (!CheckTicketFromWidget_query.exec())
        {
            qDebug() << "CheckTicketFromWidget_query last error: " +
                        QString::fromLocal8Bit( codec->fromUnicode( CheckTicketFromWidget_query.lastError().text() ) );
            //emit logger("check_ticket QUERY last error: " + check_ticket_query.lastError().text() );
        }
        while ( CheckTicketFromWidget_query.next() )
        {
            response += "<tr><td width=""8%"">";
            response += CheckTicketFromWidget_query.value(0).toString(); //GateGroup
            response += "</td><td width=""2%"">";
            response += QString::number(CheckTicketFromWidget_query.value(1).toInt()); // GateNum
            response += "</td><td width=""40%"">";
            response += CheckTicketFromWidget_query.value(2).toString(); // Action
            response += "</td><td width=""20%"">";
            response += CheckTicketFromWidget_query.value(3).toDateTime().toString(); // Time
            response += "</td></tr>";
            //qDebug() << "response: " << response;
        }
response += "</table>";


//QString tresponse =
        qDebug() << "response: " << QString::fromLocal8Bit( codec->fromUnicode( response ));
        return response;
    }
    void service(QByteArray sessionId, HttpRequest& request, HttpResponse& response)
    {
        QByteArray barcode = request.getParameter("barcode");
        //        response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");
        HttpSession session = sessionStore->getSession(sessionId);
//barcode ="superticket";
        //if (check_ticket( barcode ) > 0)
        {
              response.write( check_ticket( barcode ).toLocal8Bit());
            //response.redirect("/");
            qDebug()<<"check_pass ";
        }
      //  else {
      //      qDebug()<<"wrong";
      //  }

    }
};

#endif // DB_QUERY_PROCESSOR_H
