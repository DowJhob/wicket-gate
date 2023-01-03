#include "imodel.h"

IModel::IModel(QScxmlStateMachine *parent, statusDescriptions *desc) : QScxmlCppDataModel{parent}, machine(parent),
  desc(desc)
{

    //this->currentMethod = &IModel::sendToMainReader;
}

void IModel::certOK(QJsonArray a)
{
    certInfo = a;
    machine->submitEvent("certOK");
}

void IModel::certFail()
{
    machine->submitEvent("certFail");
}

void IModel::onMainConnected()
{
    emit s_sendTelemetry(gateState("onMainConnected"));
}

void IModel::onSlaveConnected()
{

}

void IModel::onMainDisconnected()
{
    emit s_sendTelemetry(gateState("onMainDisconnected"));
}

void IModel::onSlaveDisconnected()
{

}
// ============================================
void IModel::onEntryConnected()
{
    emit s_sendTelemetry(gateState("onEntryConnected"));
}

void IModel::onExitConnected()
{
    emit s_sendTelemetry(gateState("onExitConnected"));
}

void IModel::onEntryDisconnected()
{
    emit s_sendTelemetry(gateState("onEntryDisconnected"));
}

void IModel::onExitDisconnected()
{
    emit s_sendTelemetry(gateState("onExitDisconnected"));
}

void IModel::onArmed()
{
    emit s_sendTelemetry(gateState("onArmed"));
}
