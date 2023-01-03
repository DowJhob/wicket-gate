#include "db_worker.h"

db_worker::db_worker(QObject *parent, gateData *data, QString conn_str)://gate(gate),
    //GateCaption(data),
    data(data),
    conn_str(conn_str)
{
    gate_ptr_to_string_for_db_connection_id = QString("0x%1").arg((quintptr)parent, QT_POINTER_SIZE * 2, 16, QChar('0'));

    QThread *db_worker_thread= new QThread;

    connect(db_worker_thread, &QThread::started, this, &db_worker::create_db_connect);
    connect(this, &QObject::destroyed, db_worker_thread, &QThread::quit);
    connect(db_worker_thread, &QThread::finished, this, &db_worker::deleteLater);
    connect(db_worker_thread, &QThread::finished, db_worker_thread, &QThread::deleteLater);

    moveToThread(db_worker_thread);
    db_worker_thread->start(QThread::HighestPriority);
}

db_worker::~db_worker(){}

bool db_worker::db_open()
{
    QSqlDatabase db = QSqlDatabase::database(gate_ptr_to_string_for_db_connection_id, true);
    if ( !db.isValid() )
    {
        emit logger(GateCaption + "  db worker: QSqlDatabase::database невалидно : reader_ptr_to_string_for_db_connection_id - " + gate_ptr_to_string_for_db_connection_id);
        return false;
    }
    if ( !db.isOpen() )
    {
        emit logger(GateCaption + "  db worker: Соединение с БД закрыто,  пробуем открыть :");
        if ( !db.open() )
        {
            //   qDebug() << db.lastError();
            emit logger(GateCaption + "  db worker: Не удалось соединиться с базой данных ! :");
            return false;
        }
        //qDebug() << db.lastError();
    }
    //        emit logger("database open succefull");
    return true;
}

void db_worker::create_db_connect()
{
    //emit logger("reader_ptr_to_string_for_db_connection_id :" + reader_ptr_to_string_for_db_connection_id);
    QSqlDatabase db = QSqlDatabase::addDatabase( "QODBC3", gate_ptr_to_string_for_db_connection_id );
    db.setDatabaseName( conn_str );
    db_open();
}

void db_worker::checkCovidControler(QString barcode)
{
    if ( !db_open() )
        return;
    QSqlQuery query(QSqlDatabase::database(gate_ptr_to_string_for_db_connection_id));
    query.prepare( "SELECT count(*) FROM [TCDB_Check].[dbo].checkCovidControler WHERE [Key]="":cKey""" );
    query.bindValue(":cKey", barcode, QSql::In );
    if (!query.exec())
    {
        emit logger("db_worker::checkCovidControler db error " + query.lastError().text() );
        return;
    }

    while ( query.next() )
    {
        if( query.value(0).toInt() > 0)
            emit covidControlerOK(barcode);
        else
            emit s_covidControlerFail();
    }
    //qDebug() << "ffff" << GateByCaption;
    //emit logger("db_worker::checkCovidControler: OK" + QString::number( GateByMAC.size() ) );
    query.finish();
}

void db_worker::checkTicket(int dir, QString barcode)
{
    //qDebug() << "db_worker::check_ticket: " << message;
    check_ticket_processing_time.start();
    if ( !db_open() )                   //пробуем открыть
        return;
    QSqlQuery check_ticket_query( QSqlDatabase::database(gate_ptr_to_string_for_db_connection_id) );
    check_ticket_query.prepare("{ call dbo.usp__CheckTicketFromGate_v3(:gateMAC, :direction, :barcode, :check_result ) }");
    check_ticket_query.bindValue(":gateMAC", data->MainMAC, QSql::In );
    check_ticket_query.bindValue(":direction", dir, QSql::In );
    check_ticket_query.bindValue(":barcode", barcode, QSql::In );

    int check_result = 0x2000;
    check_ticket_query.bindValue(":check_result", check_result, QSql::Out);
    if (!check_ticket_query.exec())
    {
        emit logger("check_ticket QUERY last error: " + check_ticket_query.lastError().text() );
    }
    check_result = check_ticket_query.boundValue( ":check_result" ).toInt();

    switch (check_result) {
    case 0x0000 :                                //Вход разрешен.
    case 0x0100 : emit ticketOK(barcode); break;        //Выход разрешен.
    case 0x1000 : emit ticketControllerOK(barcode); break;
    case 0x2000 : emit covidControlerOK(barcode); break;
    case 0x0001 :                                                                      // "Вход запрещен! Билет уже на мероприятии!";
    case 0x0002 :                                                                      // "Вход запрещен! Неизвестное состояние билета";
    case 0x0110 :                                                                      // "Выход запрещен! Билет еще не прошел!";
    case 0x0120 :                                                                      // "Выход запрещен! Билет уже покинул мероприятие!";
    case 0x0130 :                                                                      // "Выход запрещен! Неизвестное состояние билета!";
    case 0x0200 :                                                                      // "Турникет не зарегистрирован!";
    case 0x0300 : emit ticketFail(ticket_state_description(check_result) ); break;    // "Билет не зарегистрирован на мероприятии!";
    }

    double c = check_ticket_processing_time.nsecsElapsed();
    emit logger("check_ticket " +
                data->Caption + ", номер: " + QString::number(data->GateNumber) +
                " direction: " +
                QString::number(dir) + " " +
                barcode + " processing_time: " +
                QString::number(c/1000000, '.', 3) + "ms");
}

void db_worker::confirmPass(int dir, QString barcode)
{
    if ( !db_open() )
        return;
    confirm_pass_processing_time.start();
    int result = 0;
    //emit logger("test confirm 1:" + QString::number(direction));
    QSqlQuery confirm_pass_query( QSqlDatabase::database(gate_ptr_to_string_for_db_connection_id) );
    confirm_pass_query.prepare("{ call dbo.usp__ConfirmPass_v2( :gateMAC, :direction, :barcode, :result) }");
    confirm_pass_query.bindValue(":gateMAC", data->MainMAC, QSql::In );
    confirm_pass_query.bindValue(":direction", dir, QSql::In );
    confirm_pass_query.bindValue(":barcode", barcode, QSql::In );
    confirm_pass_query.bindValue(":result", result, QSql::Out);

    if (!confirm_pass_query.exec())
        emit logger("confirm_pass query last error - " + confirm_pass_query.lastError().text() );

    result = confirm_pass_query.boundValue( ":result" ).toInt();
    //emit logger("test confirm 2:" + QString::number(direction));
    emit count(result);
    double c = confirm_pass_processing_time.nsecsElapsed();
    emit logger("confirm_pass " +
                data->Caption + ", номер: " + QString::number(data->GateNumber)
                + " direction: " +
                QString::number(dir) + " " +
                barcode + " processing_time: " +
                QString::number(c/1000000, '.', 3) + "ms");
}

void db_worker::_register(QString MAC, QString IP)
{
    //logger("register MAC - " + MAC );
    if ( !db_open() )
        return;
    QSqlQuery query("{ call dbo.usp__RegisterGate__new( :x ) }", QSqlDatabase::database(MAC));
    QString xml = "<gateIP>" + IP + "</gateIP><gateMAC>" + MAC + "</gateMAC>";
    //        query.prepare("{ call dbo.usp__RegisterGate( :x ) }");
    query.bindValue(":x", xml, QSql::In );
    if (!query.exec())
        emit logger("register - last error - " + query.lastError().text() );
}

QString db_worker::ticket_state_description(int desc)
{
    switch(desc)
    {
    case 0x1000: return "Слава Хозяину!";
    case 0x0200: return "Турникет не зарегистрирован!";
    case 0x0300: return "Билет не зарегистрирован на мероприятии!";
    case 0x0100: return "Выход разрешен.";
    case 0x0110: return "Выход запрещен! Билет еще не прошел!";
    case 0x0120: return "Выход запрещен! Билет уже покинул мероприятие!";
    case 0x0130: return "Выход запрещен! Неизвестное состояние билета!";
    case 0x0000: return "Вход разрешен.";
    case 0x0001: return "Вход запрещен! Билет уже на мероприятии!";
    case 0x0002: return "Вход запрещен! Неизвестное состояние билета";
    default :
        return "Неизвестный статус!";
    }
}

void db_worker::slot_timeout()
{

}
