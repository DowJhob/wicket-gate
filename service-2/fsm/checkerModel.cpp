#include "checkerModel.h"

checkerModel::checkerModel(QScxmlStateMachine *parent, statusDescriptions *desc) : IModel{parent, desc}
{
    connect(&CertChecker, &covidCertChecker::certOK, this, &checkerModel::certOK);
    connect(&CertChecker, &covidCertChecker::certFail, this, &checkerModel::certFail);
}

void checkerModel::onLocEnBc()
{
    lockedEntryBarcode = entryBarcode;
}

void checkerModel::onLocExBc()
{
    lockedEntryBarcode = exitBarcode;
}

void checkerModel::checkType(QString barcode)
{
    if(lockedEntryBarcode.contains("https://") && lockedEntryBarcode.contains(gosuslugiHost) && lockedEntryBarcode.contains("covid-cert"))
        machine->submitEvent("isCovidQR", QVariant(lockedEntryBarcode));
    else
        machine->submitEvent("noCert");
}

void checkerModel::onMainConnected()
{
    IModel::onMainConnected();
    emit sendToEntryReader(message(MachineState::undef, command::setUnlock));
}

void checkerModel::onUnlocked()
{
    emit s_sendTelemetry(gateState("onUnlocked"));
}

// ========================================================================================================

void checkerModel::onReady()
{
    emit sendToEntryReader(message(MachineState::undef, command::setLampOff));
    emit sendToExitReader(message(MachineState::undef, command::setLampOff));
    emit sendToEntryReader(message(MachineState::undef, command::showPlaceTicketStatus, desc->PlaceCovidQR));
    emit sendToExitReader(message(MachineState::undef, command::showPlaceTicketStatus, desc->PlaceCovidQR));

    emit s_sendTelemetry(gateState("onReady"));
}

void checkerModel::onCheckCovid()
{
    emit sendToEntryReader(message(MachineState::undef, command::showCheckStatus, desc->checkCovidQR));
    emit sendToExitReader(message(MachineState::undef, command::showCheckStatus, desc->checkCovidQR));
    CertChecker.getCertCheck(lockedEntryBarcode);
}

void checkerModel::onWaitCovidContoller()
{
    emit sendToEntryReader(message(MachineState::undef, command::showInfoStatus, QVariant(certInfo.toVariantList())));
    emit sendToExitReader(message(MachineState::undef, command::showInfoStatus, QVariant(certInfo.toVariantList())));
    emit sendToEntryReader(message(MachineState::undef, command::setGreenLampOn));
    emit sendToExitReader(message(MachineState::undef, command::setGreenLampOn));
}

// ========================================================================================================

void checkerModel::onShowNoTicket()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->CovidQRFail));
    emit sendToExitReader(message(MachineState::undef, command::showFailStatus, desc->CovidQRFail));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));
    emit sendToExitReader(message(MachineState::undef, command::setRedLampOn));
    emit s_sendTelemetry(gateState("wrong", desc->CovidQRFail));
}

void checkerModel::onShowCovidCheckCommunicationTimeout()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->gosUslugiFail));
    emit sendToExitReader(message(MachineState::undef, command::showFailStatus, desc->gosUslugiFail));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));
    emit sendToExitReader(message(MachineState::undef, command::setRedLampOn));
    emit s_sendTelemetry(gateState("wrong", desc->gosUslugiFail));
}

void checkerModel::onShowCovidFail()
{
    emit sendToEntryReader(message(MachineState::undef, command::showFailStatus, desc->CovidQRFail));
    emit sendToExitReader(message(MachineState::undef, command::showFailStatus, desc->CovidQRFail));
    emit sendToEntryReader(message(MachineState::undef, command::setRedLampOn));
    emit sendToExitReader(message(MachineState::undef, command::setRedLampOn));
    emit s_sendTelemetry(gateState("wrong", desc->CovidQRFail));
}
