#include "wicketModel.h"

wicketModel::wicketModel(QScxmlStateMachine *parent, statusDescriptions *desc) : IModel{parent, desc}
{
    connect(&CertChecker, &covidCertChecker::certOK, this, &wicketModel::certOK);
    connect(&CertChecker, &covidCertChecker::certFail, this, &wicketModel::certFail);
}

//void wicketModel::onMainConnected()
//{
//    IModel::onMainConnected();
//    // Запросим текущее состояние локера
//    emit sendToMainReader(message(MachineState::undef, command::getState));
//}

void wicketModel::onMainDisconnected()
{
    IModel::onMainDisconnected();
    emit sendToSlaveReader(message(MachineState::undef, command::showServiceStatus, desc->MainNotReady));
}

void wicketModel::onUnlocked()
{
    emit s_sendTelemetry(gateState("onUnlocked"));
    emit sendToEntryReader(message(MachineState::undef, command::showOpenStatus, desc->pleaseComeThrough));
    emit sendToExitReader(message(MachineState::undef, command::showOpenStatus, desc->pleaseComeThrough));
    emit sendToEntryReader(message(MachineState::undef, command::setGreenLampOn));
    emit sendToExitReader(message(MachineState::undef, command::setGreenLampOn));
}

void wicketModel::setArmed()
{
    emit sendToMainReader(message(MachineState::undef, command::setArmed));
}

void wicketModel::setUnlocked()
{
    emit sendToMainReader(message(MachineState::undef, command::setUnlock));
}

// ========================================================================================================

void wicketModel::resetEntryDoubleScan()
{
    //qDebug() << "wicketModel::resetDoubleScan" << entryBarcode;
    emit s_resetDoubleScan(lockedEntryBarcode);
}

void wicketModel::resetExitDoubleScan()
{
    //qDebug() << "wicketModel::resetExitDoubleScan" << lockedExitBarcode;
    emit s_resetDoubleScan(lockedExitBarcode);
}

void wicketModel::onReady()
{
    emit sendToEntryReader(message(MachineState::undef, command::setLampOff));
    emit sendToExitReader(message(MachineState::undef, command::setLampOff));
    emit sendToEntryReader(message(MachineState::undef, entryReadyShow, entryReadyDesc));
    emit sendToExitReader(message(MachineState::undef, command::showPlaceTicketStatus, desc->PlaceTicket));
    emit s_sendTelemetry(gateState("onReady"));
}

void wicketModel::onEntry()
{
    emit sendToExitReader(message(MachineState::undef, command::showFailStatus, desc->pleaseWaitCounterWay));
    emit sendToExitReader(message(MachineState::undef, command::setRedLampOn));
}

void wicketModel::onCheckEntryDoubleScan()
{
    lockedEntryBarcode = entryBarcode;
   // qDebug() << "wicketModel::onCheckEntryDoubleScan" << entryBarcode;
    emit sendToEntryReader(message(MachineState::undef, command::showCheckStatus, desc->checkDoubleScan));
    emit sendToEntryReader(message(MachineState::undef, command::setAlarm));
    emit s_checkDoubleScan(lockedEntryBarcode);
}

void wicketModel::onCheckTypeBarcode()
{
    //currentMethod = checkCert();
    if(lockedEntryBarcode.contains("https://") && lockedEntryBarcode.contains(gosuslugiHost) && lockedEntryBarcode.contains("covid-cert"))
    {
        //qDebug() << "wicketModel::checkCertType isCovidQR" << entryBarcode;
        machine->submitEvent("isCovidQR", lockedEntryBarcode);
    }
    else
    {
        //qDebug() << "wicketModel::checkCertType isNotCert" << entryBarcode;
        //machine->submitEvent("certTypeFail", entryBarcode);
        emit s_checkTicket(1, lockedEntryBarcode);
    }
}

void wicketModel::onCheckCert()
{
    emit sendToEntryReader(message(MachineState::undef, command::showCheckStatus, desc->checkCovidQR));
    CertChecker.getCertCheck(lockedEntryBarcode);
}

void wicketModel::onWaitCovidContoller()
{
    emit sendToEntryReader(message(MachineState::undef, command::showInfoStatus, QVariant(certInfo.toVariantList())));
}

void wicketModel::checkTicket()
{
    emit sendToEntryReader(message(MachineState::undef, command::showCheckStatus, desc->checkTicket));
    emit s_checkTicket(1, lockedEntryBarcode);
}

void wicketModel::onWaitEntryTicket()
{
    emit sendToEntryReader(message(MachineState::undef, command::showPlaceTicketStatus, desc->PlaceTicket));    // Покажем  - Предъявите билет
}

void wicketModel::onPassEntry()
{
    emit sendToMainReader(message(MachineState::undef, command::setEntryOpen));

    emit sendToEntryReader(message(MachineState::undef, command::showOpenStatus, desc->pleaseComeThrough));
    emit sendToEntryReader(message(MachineState::undef, command::setGreenLampOn));
    emit sendToEntryReader(message(MachineState::undef, command::setAlarm));          // Еще раз бибипнем

    emit s_sendTelemetry(gateState("onPassEntry"));
}

void wicketModel::onEntryDrop()
{
    emit s_sendTelemetry(gateState("onEntryDrop"));
}

void wicketModel::onEntryPassed()
{
    emit s_confirmPass(1, lockedEntryBarcode);  // Запишем проход
    emit sendToExitReader(message(MachineState::undef, command::setLampOff));
    emit sendToExitReader(message(MachineState::undef, command::showPlaceTicketStatus, desc->PlaceTicket));
    emit s_sendTelemetry(gateState("onEntryPassed"));
}

void wicketModel::onSecurityTimeout()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->pleaseWaitSecurityCheck));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));
}

// ========================================================================================================

void wicketModel::onExit()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->pleaseWaitCounterWay));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));
}

void wicketModel::onCheckExitDoubleScan()
{
    lockedExitBarcode = exitBarcode;
    emit sendToExitReader(message(MachineState::undef, command::showCheckStatus, desc->checkDoubleScan));
    emit sendToExitReader(message(MachineState::undef, command::setAlarm));
    emit s_checkDoubleScan(lockedExitBarcode);
    //emit s_sendTelemetry(gateState("onExitBarcode", exitBarcode));
}

void wicketModel::onCheckExit()
{
    emit sendToExitReader(message(MachineState::undef, command::showCheckStatus, desc->checkTicket));
    // проверка билета
    emit s_checkTicket(-1, lockedExitBarcode);
}

void wicketModel::onPassExit()
{
    emit sendToMainReader(message(MachineState::undef, command::setExitOpen));

    emit sendToExitReader(message(MachineState::undef, command::showOpenStatus, desc->pleaseComeThrough));
    emit sendToExitReader(message(MachineState::undef, command::setGreenLampOn));
    emit sendToExitReader(message(MachineState::undef, command::setAlarm));          // Еще раз бибипнем

    //emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->pleaseWaitCounterWay));
    //emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));

    emit s_sendTelemetry(gateState("onPassExit"));
}

void wicketModel::onExitPassed()
{
    emit s_confirmPass(-1, lockedExitBarcode);  // Запишем проход
    //emit s_resetDoubleScan(lockedExitBarcode);   // Сбросим с проверки
    emit s_sendTelemetry(gateState("onExitPassed"));
}

void wicketModel::onExitDrop()
{
    //emit s_resetDoubleScan(lockedExitBarcode);
    emit s_sendTelemetry(gateState("onExitDrop"));
}

// ====================================================================================================

void wicketModel::onShowCertFirst()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->certFirst));
    emit s_sendTelemetry(gateState("wrong", desc->certFirst));
}

void wicketModel::onShowCertCtrlFirst()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->certCtrlFirst));
    emit s_sendTelemetry(gateState("wrong", desc->certCtrlFirst));
}

void wicketModel::onShowEntryDoubleScanFail()
{
    QString s = this->desc->doubleScanFail + failDesc;
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, s));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));

    emit s_sendTelemetry(gateState("wrong", s));
}

void wicketModel::onShowEnDbTimeout()
{
   // emit s_resetDoubleScan(entryBarcode);
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->DbFail));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));

    emit s_sendTelemetry(gateState("wrong", desc->DbFail));
}

void wicketModel::onShowEntryTicketFail()
{
    //QString ss = desc->TicketFail + entryBarcode;
    //emit s_resetDoubleScan(1, entryBarcode);
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, failDesc));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));

    emit s_sendTelemetry(gateState("wrong", failDesc));
}

void wicketModel::onShowEnCertCtrlFail()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->CertCtrlFail));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));
    emit s_sendTelemetry(gateState("wrong", desc->CertCtrlFail));
}

void wicketModel::onShowCovidContollerFail()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->CovidControlerQR_Fail));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));
    emit s_sendTelemetry(gateState("wrong", desc->CovidControlerQR_Fail));
}

void wicketModel::onShowCovidCheckCommunicationTimeout()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->gosUslugiFail));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));
    emit s_sendTelemetry(gateState("wrong", desc->gosUslugiFail));
}

void wicketModel::onShowCovidFail()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->CovidQRFail));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));
    emit s_sendTelemetry(gateState("wrong", desc->CovidQRFail));
}

// ====================================================================================================

void wicketModel::onShowExCertCtrlFail()
{
    //emit s_resetDoubleScan(lockedExitBarcode);   // Сбросим с проверки
    emit sendToExitReader(message(MachineState::undef, command::showFailStatus, desc->CertCtrlFail));
    emit sendToExitReader(message(MachineState::undef, command::setRedLampOn));

    emit s_sendTelemetry(gateState("wrong", desc->CertCtrlFail));
}

void wicketModel::onShowExitDbFail()
{
    //emit s_resetDoubleScan(lockedExitBarcode);
    emit sendToExitReader(message(MachineState::undef, command::showFailStatus, desc->DbFail));
    emit sendToExitReader(message(MachineState::undef, command::setRedLampOn));

    emit s_sendTelemetry(gateState("wrong", desc->DbFail));
}

void wicketModel::onShowExitDoubleScanFail()
{
    QString s = this->desc->doubleScanFail + failDesc;
    emit sendToExitReader(message(MachineState::undef, command::showFailStatus, s));
    emit sendToExitReader(message(MachineState::undef, command::setRedLampOn));

    emit s_sendTelemetry(gateState("wrong", s));
}

void wicketModel::onShowExitTicketFail()
{
    //QString s = desc->TicketFail + exitBarcode;
    //emit s_resetDoubleScan(lockedExitBarcode);
    emit sendToExitReader(message(MachineState::undef, command::showFailStatus, failDesc));
    emit sendToExitReader(message(MachineState::undef, command::setRedLampOn));

    emit s_sendTelemetry(gateState("wrong", failDesc));
}
