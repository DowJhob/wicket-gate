#include "main_server.h"

main_server::main_server(QObject *parent, QSettings *settings) : QObject(parent), settings(settings)
{
    getSettings(settings);
}

main_server::~main_server()
{
}

void main_server::start()
{
    create_db_connect(conn_str);
    get_Gate_data_from_DB();
    get_ticket_pass_count();
    emit logger("Main server started:");
}

void main_server::getSettings(QSettings *settings)
{
    QString def =  "Driver={ODBC Driver 17 for SQL Server};"
                   "Server=lpc:" + QHostInfo::localHostName() + "\\SQLEXPRESS;Database=TCDB_Check;"//              "Port=1433;"
            "Uid=TC;Pwd=tc12452904;"
            //QString def = "Driver={SQL Server};Server=10.1.7.11;Database=TCDB_Check;Port=1433;Uid=TC;Pwd=tc12452904";
            "CharSet=UTF16;Trusted_Connection=no;SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_WRITE_ONLY;"
            //"SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
            "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC4;"   // ниже 4 косячит
            "SQL_ATTR_CONNECTION_TIMEOUT=2;SQL_ATTR_LOGIN_TIMEOUT=2;";
    //=====================-- --==================================
    settings->beginGroup( "db" );
    conn_str = settings->value( "connection_string", def ).toString();// + QHostInfo::localHostName() + settings->value( "connection_string2" ).toString();
    if ( !( settings->contains("connection_string")) )
        emit logger("В конфиг файле отсутствует строка подключения, использовано значение по умолчанию!!\n");
    settings->endGroup();
    emit logger("Строка подключения к БД = \n" + conn_str);

    //       settings->beginGroup( "client" );
    //    int reconnectInterval = settings->value( "reconnectInterval", 20000 ).toInt();// + QHostInfo::localHostName() + settings->value( "connection_string2" ).toString();

    //      settings->endGroup();
    //=====================-- --==================================
    settings->beginGroup( "adm_function" );

    unlockCard = settings->value( "unlockCard", "" ).toString();
    armedCard = settings->value( "armedCard", "" ).toString();

    settings->endGroup();

}

void main_server::create_db_connect(QString conn_str)
{
    db = QSqlDatabase::addDatabase( "QODBC3", "DB" );
    db.setDatabaseName( conn_str );
    db_open();
}

bool main_server::db_open()
{
    if ( !db.isOpen() )
        if ( !db.open() )
        {
            emit logger("database not open");
            return false;
        }
    //        emit logger("database open succefull");
    return true;
}

void main_server::get_Gate_data_from_DB()
{
    if ( !db_open() )
        return;
    QSqlQuery query(QSqlDatabase::database( "DB" ));
    query.prepare( "SELECT Caption, GateNumber, MainMAC, SlaveMAC, EntryMAC, ExitMAC, MainIP, SlaveIP, standAlone, certCheck FROM dbo.Gate2" );
    if (!query.exec())
        emit logger("get gate data from db error " + query.lastError().text() );

    while ( query.next() )
    {
        gateData gd{query.value(0).toString(), query.value(1).toInt(), query.value(2).toString(), query.value(3).toString(),
                    query.value(4).toString(), query.value(5).toString(), query.value(6).toString(), query.value(7).toString(), query.value(8).toBool(), query.value(9).toBool()};

        //emit logger("/////////////////////////////////////////////////////////////////////////////////////////////////////////////////" + QString::number(gd.standAlone));
        wicketGate *wg = new wicketGate(&wicketGateListByBarcode, gd, conn_str, this);

        connect(wg, &wicketGate::s_checkDoubleScan, this, &main_server::checkDoubleScan);
        connect(wg, &wicketGate::s_resetDoubleScan, this, &main_server::m_resetDoubleScan);
        connect(wg, &wicketGate::logger, this, &main_server::logger);
        connect(wg, &wicketGate::count, this, &main_server::ticketCounter);

        QString MainMAC = query.value(2).toString();
        if(!MainMAC.isEmpty())
        {
            GateByMAC.insert( MainMAC, wg );
            GateByCaption.insert(gd.Caption + ':' + QString::number(gd.GateNumber), wg);
            //emit logger("АхтунгАхтунгАхтунг " + gd.Caption);
        }
        else
            emit logger("Ахтунг, у записи в базе нет главного МАК адреса: " + QString::number( GateByMAC.size() ) );
        QString SlaveMAC = query.value(3).toString();
        if(!SlaveMAC.isEmpty())
            GateByMAC.insert( SlaveMAC, wg );
    }
    //qDebug() << "ffff" << GateByCaption;
    emit logger("количество MAC адресов считывателей в БД: " + QString::number( GateByMAC.size() ) );
    query.finish();
}

void main_server::ticketCounter(int dir)
{
    current_count += dir;
    if (current_count < 0)
    {
        current_count = 0;
        emit logger("Ахтунг! Выходят неучтенные посетители!" );
    }
    if(dir > 0)
    {
        total_count += dir;
    }
}

int main_server::register_gate_in_db(QString MAC, QString IP)
{
    //logger("register MAC - " + MAC );
    if ( !db_open() )
        return -1;
    QSqlQuery query(db);
    QString xml = "<gateIP>" + IP + "</gateIP><gateMAC>" + MAC + "</gateMAC>";
    query.prepare("{ call dbo.usp__RegisterGate_v2( :gateIP, :gateMAC , :result ) }");
    int result = 0;
    query.bindValue(":gateMAC", MAC, QSql::In );
    query.bindValue(":gateIP", IP, QSql::In );
    query.bindValue(":result", result, QSql::Out);
    if (!query.exec())
    {
        emit logger("Ошибка регистрации нового считывателя - " + query.lastError().text() );
        return -1;
    }
    result = query.boundValue( ":result" ).toInt();
    return result;
}

void main_server::newReaderConnected2(int desc)
{
        readerConnector *reader = new readerConnector(desc, 10000);
        connect(reader, &readerConnector::recieveMAC, this, &main_server::registerWicketReaderByMAC//, Qt::QueuedConnection
            );
        //reader->requestMAC();
        QMetaObject::invokeMethod(reader, "requestMAC");
}

void main_server::newReaderConnected3(readerConnector *reader)
{
        connect(reader, &readerConnector::recieveMAC, this, &main_server::registerWicketReaderByMAC//, Qt::QueuedConnection
                );
        //reader->requestMAC();
        QMetaObject::invokeMethod(reader, "requestMAC");
}

void main_server::registerWicketReaderByMAC(readerConnector* s, QString MAC)
{
        wicketGate *gate = GateByMAC.value(MAC, nullptr);          // Найдем гейт с таким маком в контейнере

    if(gate != nullptr)
    {
        gate->setReader(MAC, s);

        connect(gate, &wicketGate::sig_gateTelemetry, this, &main_server::sendTelemetry );
        emit logger("Группа: " + gate->data.Caption + ", номер: " + QString::number(gate->data.GateNumber) + ", MAC: " + MAC );
    }
    else
    {
        emit logger("Зарегистрирован новый считыватель: MAC: " + MAC + " IP: " + s->addr);
        register_gate_in_db(MAC, s->addr);
    }
}

void main_server::checkDoubleScan(QString barcode)
{
    wicketGate *senderGate = static_cast<wicketGate*>(sender());

    wicketGate *currentGate = wicketGateListByBarcode.value(barcode, nullptr);
    //QString num = senderGate->data.Caption + ", номер: " + QString::number(senderGate->data.GateNumber);
    if(doubleScanSwitcher && currentGate != nullptr)
    {
        QString description = "Первым был " + currentGate->data.Caption + ", номер: " + QString::number(currentGate->data.GateNumber);
        emit logger( senderGate->data.Caption + ", номер: " + QString::number(senderGate->data.GateNumber) + "\t |" + description +" -  " + barcode);
        senderGate->machine->submitEvent("fail.dblscn");
        senderGate->wicketmodel->failDesc = description;
        //qDebug() << "main_server::checkDoubleScan fail" << description;
    }
    else
    {
        wicketGateListByBarcode.insert(barcode, senderGate);
        senderGate->machine->submitEvent("ok.dblscn", barcode);
        //qDebug() << "main_server::checkDoubleScan ok" << barcode;
    }
}

void main_server::m_resetDoubleScan(QString barcode)
{
    wicketGateListByBarcode.remove(barcode);
}

void main_server::slot_widget_readyRead(QString str)
{
    qDebug()<<"from widget : " << str;
    if(str == "getState")
    {
        for(auto gate : qAsConst(GateByMAC))
        {
            //qDebug()<<"activeStateNames:" << gate->data.Caption << gate->data.GateNumber << gate->machine->activeStateNames(false);
            for(const auto &state : gate->machine->activeStateNames(false))
            {
                gate->gateT["state"] = state;
                sendTelemetry(gate->gateT);
            }
        }
        return;
    }
    if(str == "setIron")
    {
        doubleScanSwitcher = false;
        return;
    }
    if(str == "setNormal")
    {
        doubleScanSwitcher = false;
        return;
    }
    if(str == "dblScnOff")
    {
        doubleScanSwitcher = false;
        return;
    }
    if(str == "dblScnOn")
    {
        doubleScanSwitcher = true;
        return;
    }
    if(str == "dblScnClr")
    {
        wicketGateListByBarcode.clear();
        return;
    }

    if(str == "rstCntrs")
    {
        //qDebug()<<"from widget resetCounters: " << str;
        current_count = 0;
        total_count = 0;
        return;
    }



//    if(str == "sync")
//    {
//        qDebug()<<"from widget resetCounters: " << str;
//        current_count = 0;
//        total_count = 0;
//        return;
//    }
//    if(str == "rstCntrs")
//    {
//        qDebug()<<"from widget resetCounters: " << str;
//        current_count = 0;
//        total_count = 0;
//        return;
//    }

    QStringList cmd_list = str.split('\n');
    for ( const QString& command : qAsConst(cmd_list) )
    {
        QStringList command_field_list = command.split(';');
        int count = command_field_list.count();
        message m;
        if (count < 2)
            continue;
        wicketGate *gate = GateByCaption.value( command_field_list[0], nullptr );
        if (gate != nullptr)
        {
            auto ccc = ccccc.value(command_field_list[1]);
            ccc(gate);
        }
    }
}

void main_server::get_ticket_pass_count()
{
    if ( !db_open() )
        return ;
    QSqlQuery query(db);
    query.prepare("{ call dbo.get_ticket_pass_count }");
    if (!query.exec())
        emit logger("get ticket pass count from db error " + query.lastError().text() );

    while ( query.next() )
    {
        current_count += query.value(1).toInt();
        total_count += query.value(2).toInt();
        ticket_pass_data.insert(query.value(0).toString(), {query.value(1).toInt(), query.value(2).toInt()});
    }
    //      send_to_widgets( "count=" + QString::number(current_count) + ":" + QString::number(total_count));
    //       emit logger("количество турникетов в БД: " + QString::number( Gate_data_from_DB.size() ) );
    query.finish();
}

void main_server::sendTelemetry(QJsonObject json)
{//Прокладка для добавления к телеметрии счетчика проходов
    json["current_count"] = current_count;
    json["total_count"] = total_count;
    emit send_to_widgets2( QJsonDocument(json).toJson(QJsonDocument::Compact) );
}
