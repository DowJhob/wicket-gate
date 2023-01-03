#ifndef IMODEL_H
#define IMODEL_H

#include <QScxmlStateMachine>
#include <QScxmlCppDataModel>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

#include "common_types.h"
#include "command.h"

#include "../covidcertchecker.h"

class IModel : public QScxmlCppDataModel
{
    Q_OBJECT
public:
    QString showTime{"2s"};
    QString WaitCovidContollerTime{"30s"};
    QString dbTimeout{"5s"};
    QString passTime{"10s"};
    QString CheckTicketTime{"30s"};
    QString certTimeout{"5s"};
    QString SecurityCheckTime{"5s"};

    QScxmlStateMachine *machine = nullptr;
    QJsonArray certInfo;

    QString entryBarcode{};
    QString exitBarcode{};
    QString lockedEntryBarcode{};
    QString lockedExitBarcode{};
    QString failDesc{};

    command entryReadyShow = command::showPlaceCertStatus;
    QString entryReadyDesc;             // Только на вход нужно менять и держать текущую, на выход всегда Place ticket
    //QString exitReadyDesc;
    bool certCheckEnable = true;

    QString gosuslugiHost = "gosuslugi.ru";
    statusDescriptions *desc;
    covidCertChecker CertChecker;

    QJsonObject gateTelemetryData{
        {"caption", ""},
        {"number", ""}
        //{"temp",""},
        //{"wicket_state", 0},
        //{"barcode",""},
        //{"total_count",0},
        //{"current_count",0}
    };

    //void (IModel::*currentMethod)(void*);

    explicit IModel(QScxmlStateMachine *parent = nullptr, statusDescriptions *desc = nullptr);

    virtual void certOK(QJsonArray a);

    virtual void certFail();

    virtual void onMainConnected();
    virtual void onSlaveConnected();
    virtual void onMainDisconnected();
    virtual void onSlaveDisconnected();

    virtual void onEntryConnected();
    virtual void onExitConnected();
    virtual void onEntryDisconnected();
    virtual void onExitDisconnected();

    virtual void onArmed();

signals:
    void sendToMainReader(message);  // to main reader
    void sendToSlaveReader(message);  // to main reader
    void sendToEntryReader(message);  // to Entry reader
    void sendToExitReader(message);  // to Exit reader

    void s_checkDoubleScan(QString);
    void s_resetDoubleScan(QString);
    void s_checkTicket(int, QString);
    void s_confirmPass(int, QString);

    void s_sendTelemetry(gateState);   //
};

#endif // IMODEL_H
