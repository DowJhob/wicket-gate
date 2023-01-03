#ifndef WICKETGATE_H
#define WICKETGATE_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>

#include "checker.h"
#include "locker.h"   // Не забудь что это говно рожает комилятор из твоего scxml..

#include "reader-connector.h"
#include "fsm/scxmlexternmonitor2.h"
#include "db_worker.h"
#include "fsm/wicketModel.h"
#include "fsm/checkerModel.h"

class wicketGate : public QObject
{
    Q_OBJECT
public:
    QScxmlStateMachine *machine = nullptr;
    IModel *wicketmodel = nullptr;
    //main *machine = nullptr;

    gateData data;
    statusDescriptions desc;

    readerConnector *mainReader = nullptr;
    readerConnector *slaveReader = nullptr;
    readerConnector *entryReader = nullptr;
    readerConnector *exitReader = nullptr;

    QJsonObject gateT{
        {"caption", ""},
        {"number", ""}
        //        {"temp",""},
        //        {"wicket_state", 0},
        //        {"barcode",""},
        //        {"total_count",0},
        //        {"current_count",0}
    };

    Scxmlmonitor::UDPScxmlExternMonitor *monitor = nullptr;

    explicit wicketGate(QHash<QString, wicketGate*> *wicketGateListByBarcode, gateData data, QString conn_str, QObject *parent = nullptr);
    ~wicketGate();

    // тут сразу селекция кто вход кто выход и ведущий-подчиненный
    void setReader(QString MAC, readerConnector *Reader);

    ///
    /// \brief setArmed Взвести турникет
    ///
    void setArmed();
    ///
    /// \brief setUnLocked Разблокировать турникет
    ///
    void setUnLocked();
    void stndAlnOn();
    void stndAlnOff();
    void certOn();
    void certOff();

public slots:
    void readerMainDisconnected();
    void readerSlaveDisconnected();

private slots:
    void mainCMD_Parse(message msg);
    void slaveCMD_Parse(message msg);
    void entryCMD_Parse(message msg);
    void exitCMD_Parse(message msg);
    void sendTelemetry(gateState gs);
    void createFSM();

    void dbWorker(gateData *data, QString conn_str);
    void createCheckerFSM();

private:
    db_worker *_db_worker = nullptr;

    QString gosuslugiHost = "gosuslugi.ru";
    QHash<QString, wicketGate*> *wicketGateListByBarcode = nullptr;


    QString conn_str;
    QJsonDocument gateCaptionJSON;

    //template <class T>
    void resetCurrentW();
    void resetCurrentC();

    typedef void (checkerModel::*cfunc)();
    QHash<QString, cfunc> cexec{
        {"onReady", &checkerModel::onReady},
        {"onCheckCovid", &checkerModel::onCheckCovid},
        {"onWaitCovidContoller", &checkerModel::onWaitCovidContoller},
        {"onShowNoTicket", &checkerModel::onShowNoTicket},
        {"onShowCovidCheckCommunicationTimeout", &checkerModel::onShowCovidCheckCommunicationTimeout},
        {"onShowCovidFail", &checkerModel::onShowCovidFail}
    };


    typedef void (wicketModel::*func)();
    QHash<QString, func> exec{
        {"onMainDisconnected", &wicketModel::onMainDisconnected},
        {"onUnlocked", &wicketModel::onUnlocked},
        {"setArmed", &wicketModel::setArmed},
        {"setUnlocked", &wicketModel::setUnlocked},
        {"onArmed", &wicketModel::onArmed},

        {"resetEntryDoubleScan", &wicketModel::resetEntryDoubleScan},
        {"resetExitDoubleScan", &wicketModel::resetExitDoubleScan},

        {"onEntry", &wicketModel::onEntry},
        {"onReady", &wicketModel::onReady},
        {"onCheckEntryDoubleScan", &wicketModel::onCheckEntryDoubleScan},
        {"onCheckTypeBarcode", &wicketModel::onCheckTypeBarcode},
        {"onCheckCert", &wicketModel::onCheckCert},
        {"onWaitCovidContoller", &wicketModel::onWaitCovidContoller},
        {"checkTicket", &wicketModel::checkTicket},
        {"onWaitEntryTicket", &wicketModel::onWaitEntryTicket},
        {"onPassEntry", &wicketModel::onPassEntry},
        {"onEntryDrop", &wicketModel::onEntryDrop},
        {"onEntryPassed", &wicketModel::onEntryPassed},
        {"onSecurityTimeout", &wicketModel::onSecurityTimeout},

        {"onExit", &wicketModel::onExit},
        {"onCheckExitDoubleScan", &wicketModel::onCheckExitDoubleScan},
        {"onCheckExit", &wicketModel::onCheckExit},
        {"onPassExit", &wicketModel::onPassExit},
        {"onExitDrop", &wicketModel::onExitDrop},
        {"onExitPassed", &wicketModel::onExitPassed},

        {"onShowCertFirst", &wicketModel::onShowCertFirst},
        {"onShowCertCtrlFirst", &wicketModel::onShowCertCtrlFirst},
        {"onShowEntryDoubleScanFail", &wicketModel::onShowEntryDoubleScanFail},
        {"onShowExitDoubleScanFail", &wicketModel::onShowExitDoubleScanFail},
        {"onShowEnDbTimeout", &wicketModel::onShowEnDbTimeout},
        {"onShowExitDbFail", &wicketModel::onShowExitDbFail},
        {"onShowEntryTicketFail", &wicketModel::onShowEntryTicketFail},

        {"onShowEnCertCtrlFail", &wicketModel::onShowEnCertCtrlFail},

        {"onShowExitTicketFail", &wicketModel::onShowExitTicketFail},
        {"onShowExCertCtrlFail", &wicketModel::onShowExCertCtrlFail},
        {"onShowCovidContollerFail", &wicketModel::onShowCovidContollerFail},
        {"onShowCovidCheckCommunicationTimeout", &wicketModel::onShowCovidCheckCommunicationTimeout},
        {"onShowCovidFail", &wicketModel::onShowCovidFail},
    };


signals:
    /// \brief Сигнал проверки на двойной проход
    void s_checkDoubleScan(QString);
    void s_resetDoubleScan(QString);
    void sig_gateTelemetry(QJsonObject);
    void logger(QString);

    void count(int);

};

#endif // WICKETGATE_H
