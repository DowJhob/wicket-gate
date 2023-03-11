#include "wicketgate.h"

wicketGate::wicketGate(QHash<QString, wicketGate *> *wicketGateListByBarcode, gateData data, QString conn_str, QObject *parent)
    : QObject(parent), data(data), wicketGateListByBarcode(wicketGateListByBarcode), conn_str(conn_str)
{
    gateT["caption"] = data.Caption;
    gateT["number"] = data.GateNumber;
    gateT["IP"] = data.MainIP;

    if(data.standAlone)
        createCheckerFSM();
    else
        createFSM();

    wicketmodel->certCheckEnable = data.certCheck;

    machine->start();
}

wicketGate::~wicketGate()
{
    monitor->deleteLater();
}

void wicketGate::setReader(QString MAC, readerConnector *Reader)
{
    if(MAC == data.EntryMAC)
    {
        entryReader = Reader;
        connect(entryReader, &readerConnector::readerDisconnected, this, [this](){
            machine->submitEvent("entryDisconnected");
            entryReader = nullptr;} );
        connect(entryReader, &readerConnector::read, this, &wicketGate::entryCMD_Parse ); // Примем комманду от входного
        connect(wicketmodel, &wicketModel::sendToEntryReader, entryReader, &readerConnector::send_to_socket );
        machine->submitEvent("entryConnected");
    }
    else if(MAC == data.ExitMAC)
    {
        exitReader = Reader;
        connect(exitReader, &readerConnector::readerDisconnected, this, [this](){
            machine->submitEvent("exitDisconnected");
            exitReader = nullptr;} );
        connect(exitReader, &readerConnector::read, this, &wicketGate::exitCMD_Parse );
        connect(wicketmodel, &wicketModel::sendToExitReader, exitReader, &readerConnector::send_to_socket );

        machine->submitEvent("exitConnected");
    }
    else
        emit logger("wicketGate::setReader / не найдены входной и выходной маки");

    if(MAC == data.MainMAC)
    {
        mainReader = Reader;
        connect(mainReader, &readerConnector::readerDisconnected, this, &wicketGate::readerMainDisconnected );
        connect(mainReader, &readerConnector::read, this, &wicketGate::mainCMD_Parse );                    // Примем команды и статусы именно от майна
        connect(wicketmodel, &wicketModel::sendToMainReader, mainReader, &readerConnector::send_to_socket );  // Отправим команды КА мэйну
        machine->submitEvent("mainConnected");
        //QMetaObject::invokeMethod( Reader, "send_to_socket", Q_ARG( message, message(MachineState::undef, command::getState) ) );
    }
    else if(MAC == data.SlaveMAC)
    {
        slaveReader = Reader;
        connect(slaveReader, &readerConnector::readerDisconnected, this, &wicketGate::readerSlaveDisconnected );
        connect(wicketmodel, &wicketModel::sendToSlaveReader, slaveReader, &readerConnector::send_to_socket );
        machine->submitEvent("slaveConnected");
    }
    else
        emit logger("wicketGate::setReader / не найдены ведущий и подчиненный маки");

    //  Обновим текущее состояние
    if(data.standAlone)
        resetCurrentC();
    else
        resetCurrentW();

    // Запросим текущее состояние локера
    emit wicketmodel->sendToMainReader(message(MachineState::undef, command::getState, data.Caption + " : " + QString::number(data.GateNumber)));
}

void wicketGate::setArmed()
{
    if(machine != nullptr)   // когда есть стэндАлоны они упадут
        machine->submitEvent("cmd.arm");
}

void wicketGate::setUnLocked()
{
    if(machine != nullptr)   // когда есть стэндАлоны они упадут
        machine->submitEvent("cmd.unlock");
}

void wicketGate::stndAlnOn()
{
    data.standAlone = true;
    if(machine != nullptr)
    {
        machine->deleteLater();
        machine = nullptr;
    }
    createCheckerFSM();
    if(mainReader != nullptr)
        setReader(data.MainMAC, mainReader);
    if(slaveReader != nullptr)
        setReader(data.SlaveMAC, slaveReader);
    if(entryReader != nullptr)
        setReader(data.EntryMAC, entryReader);
    if(exitReader != nullptr)
        setReader(data.ExitMAC, exitReader);

    machine->start();
}

void wicketGate::stndAlnOff()
{
    data.standAlone = false;
    if(machine != nullptr)
    {
        machine->deleteLater();
        machine = nullptr;
    }
    createFSM();
    if(mainReader != nullptr)
        setReader(data.MainMAC, mainReader);
    if(slaveReader != nullptr)
        setReader(data.SlaveMAC, slaveReader);
    if(entryReader != nullptr)
        setReader(data.EntryMAC, entryReader);
    if(exitReader != nullptr)
        setReader(data.ExitMAC, exitReader);

    machine->start();
}

void wicketGate::certOn()
{
    wicketmodel->certCheckEnable = true;
    wicketmodel->entryReadyDesc = desc.PlaceCovidQR;
    wicketmodel->entryReadyShow = command::showPlaceCertStatus;
    //machine->submitEvent("reset");
}

void wicketGate::certOff()
{
    wicketmodel->certCheckEnable = false;
    wicketmodel->entryReadyDesc = desc.PlaceTicket;
    wicketmodel->entryReadyShow = command::showPlaceTicketStatus;
    //machine->submitEvent("reset");
}

void wicketGate::readerMainDisconnected()
{
    machine->submitEvent("mainDisconnected");
    mainReader = nullptr;
    // Послать телеметрию
    //gateT["state"] = "disconnected";
    //emit sig_gateTelemetry(gateT);
}

void wicketGate::readerSlaveDisconnected()
{
    slaveReader = nullptr;
    // Послать телеметрию
    //gateT["state"] = "disconnected";
    //emit sig_gateTelemetry(gateT);
}

void wicketGate::mainCMD_Parse(message msg)
{
    //    QString str = msg.body.toString();

    switch (msg.cmd) {
    case command::onArmed  : machine->submitEvent("crsbrd.armed"); break;
    case command::onUnlock : machine->submitEvent("crsbrd.unlocked"); break;
    case command::onPassed : machine->submitEvent("crsbrd.passed"); break;
    default                :                 ; break;
    }
}

void wicketGate::slaveCMD_Parse(message msg)
{
    // от слэйва мне ничего не нужно
    //    QString str = msg.body.toString();
    //    QString state;

    //    switch (msg.cmd) {
    //case command::getTemp    : sendTelemetry(str); break;
    //    default                  :                 ; break;
    //    }

}

void wicketGate::entryCMD_Parse(message msg)
{
    QString body = msg.body.toString();

    switch (msg.cmd) {
    case command::onBarcode :
        machine->submitEvent("en.barcode");
        wicketmodel->entryBarcode = body;
        break;
    case command::onTemp    :
        gateT["rdr_type"] = 1;
        gateT["temp"] = body;
        gateT["state"] = "temp";
        emit sig_gateTelemetry(gateT); break;
    default                  :                 ; break;
    }
}

void wicketGate::exitCMD_Parse(message msg)
{
    QString body = msg.body.toString();

    switch (msg.cmd) {
    case command::onBarcode :
        machine->submitEvent("ex.barcode", body);
        wicketmodel->exitBarcode = body;
        break;
    case command::onTemp    :
        gateT["rdr_type"] = -1;
        gateT["temp"] = body;
        gateT["state"] = "temp";
        emit sig_gateTelemetry(gateT); break;
    default                  :                 ; break;
    }
}

void wicketGate::sendTelemetry(gateState gs)
{
    gateT["state"] = gs.state;
    gateT["body"] = gs.body;
    //gateT["locker"] = gs.state;
    emit sig_gateTelemetry(gateT);
    //qDebug() << "wicketGate::sendTelemetry " << gateT;
}

void wicketGate::dbWorker(gateData *data, QString conn_str)
{
    _db_worker = new db_worker(this, data, conn_str);
    connect(_db_worker, &db_worker::logger, this, &wicketGate::logger);
    connect(wicketmodel, &wicketModel::s_checkTicket, _db_worker, &db_worker::checkTicket);
    connect(_db_worker, &db_worker::ticketOK, machine, [this](QString f){machine->submitEvent("ticketOK", f);});
    connect(_db_worker, &db_worker::ticketControllerOK, machine, [this](QString f){machine->submitEvent("ticketControllerOK", f);});
    connect(_db_worker, &db_worker::covidControlerOK, machine, [this](QString f){machine->submitEvent("covidControllerOK", f);});
    connect(_db_worker, &db_worker::ticketFail, machine, [this](QString f){
        machine->submitEvent("ticketFail");
        wicketmodel->failDesc = f;
    });

    connect(_db_worker, &db_worker::dbFail, machine, [this](){
        machine->submitEvent("dbTimeout");
        wicketmodel->failDesc = "База данных не отвечает";
    });

    connect(wicketmodel, &wicketModel::s_confirmPass, _db_worker, &db_worker::confirmPass);
    //connect(_db_worker, &db_worker::count, wicketmodel,  &wicketModel::count);
}

void wicketGate::createFSM()
{
    //wicketmodel->exitReadyDesc = desc.PlaceTicket;

    //==================================================================================================================
    machine = new Wicket(this);
    wicketmodel = new wicketModel(machine, &desc);
    machine->setDataModel(wicketmodel);
    dbWorker(&data, conn_str);

    if(data.certCheck)
    {
        wicketmodel->entryReadyDesc = desc.PlaceCovidQR;
        wicketmodel->entryReadyShow = command::showPlaceCertStatus;
    }
    else
    {
        wicketmodel->entryReadyDesc = desc.PlaceTicket;
        wicketmodel->entryReadyShow = command::showPlaceTicketStatus;
    }

    connect(wicketmodel, &wicketModel::s_confirmPass, this, [this](int dir, QString bc){emit count(dir);});
    connect(wicketmodel, &wicketModel::s_checkDoubleScan, this, &wicketGate::s_checkDoubleScan);
    connect(wicketmodel, &wicketModel::s_resetDoubleScan, this, &wicketGate::s_resetDoubleScan);
    connect(wicketmodel, &wicketModel::s_sendTelemetry, this, &wicketGate::sendTelemetry);

    monitor = new Scxmlmonitor::UDPScxmlExternMonitor(machine);
    monitor->setMachineID(gateT["caption"].toString() + ":" + QString::number(gateT["number"].toInt()));
    monitor->setRemoteHost("10.1.7.204");
    monitor->setScxmlStateMachine(machine);
    //monitor->setRemotePort(monitor->remotePort()+1);

}

void wicketGate::createCheckerFSM()
{
    //wicketmodel->exitReadyDesc = desc.PlaceCovidQR;
    machine = new checker(this);
    wicketmodel = new checkerModel(machine, &desc);
    machine->setDataModel(wicketmodel);

    wicketmodel->entryReadyDesc = desc.PlaceCovidQR;

    connect(wicketmodel, &wicketModel::s_sendTelemetry, this, &wicketGate::sendTelemetry);

    monitor = new Scxmlmonitor::UDPScxmlExternMonitor(machine);
    monitor->setMachineID(gateT["caption"].toString() + ":" + QString::number(gateT["number"].toInt()));
    monitor->setRemoteHost("10.1.7.204");
    monitor->setScxmlStateMachine(machine);

    //monitor->setRemotePort(monitor->remotePort()+1);
}

void wicketGate::resetCurrentW()
{
    auto model = static_cast<wicketModel*>(wicketmodel);
    if(machine->isRunning())
    {
        //qDebug() << "" << machine->activeStateNames();
        for(const auto &it : machine->activeStateNames())
        {
            func t = exec.value(it, nullptr);
            if(t != nullptr)
                (model->*t)();
        }
    }
}

void wicketGate::resetCurrentC()
{
    auto model = static_cast<checkerModel*>(wicketmodel);
    if(machine->isRunning())
    {
        //qDebug() << "" << machine->activeStateNames();
        for(const auto &it : machine->activeStateNames())
        {
            cfunc t = cexec.value(it, nullptr);
            if(t != nullptr)
                (model->*t)();
        }
    }
}
