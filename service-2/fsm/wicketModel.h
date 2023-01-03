#ifndef WICKETMODEL_H
#define WICKETMODEL_H

#include <QDebug>

#include "imodel.h"

class wicketModel : public IModel
{
    Q_OBJECT
    Q_SCXML_DATAMODEL
public:
    explicit wicketModel(QScxmlStateMachine *parent = nullptr, statusDescriptions *desc = nullptr);

    //void onMainConnected() Q_DECL_OVERRIDE;
    void onMainDisconnected() Q_DECL_OVERRIDE;
    void onUnlocked();
    void setArmed();
    void setUnlocked();

    void resetEntryDoubleScan();
    void resetExitDoubleScan();


    void onEntry();
    void onReady();
    void onCheckEntryDoubleScan();
    void onCheckTypeBarcode();
    void onCheckCert();
    void onWaitCovidContoller();
    void checkTicket();
    void onWaitEntryTicket();
    void onPassEntry();
    void onEntryDrop();
    void onEntryPassed();
    void onSecurityTimeout();

    void onExit();
    void onCheckExitDoubleScan();
    void onCheckExit();
    void onPassExit();
    void onExitDrop();
    void onExitPassed();

    void onShowCertFirst();
    void onShowCertCtrlFirst();
    void onShowEntryDoubleScanFail();
    void onShowExitDoubleScanFail();
    void onShowEnDbTimeout();
    void onShowExitDbFail();
    void onShowEntryTicketFail();

    void onShowEnCertCtrlFail();

    void onShowExitTicketFail();
    void onShowExCertCtrlFail();
    void onShowCovidContollerFail();
    void onShowCovidCheckCommunicationTimeout();
    void onShowCovidFail();

private slots:

private:

signals:

};

#endif // WICKETMODEL_H
