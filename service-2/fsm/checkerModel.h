#ifndef CHECKERMODEL_H
#define CHECKERMODEL_H

#include <QDebug>

#include "imodel.h"

class checkerModel : public IModel
{
    Q_OBJECT
    Q_SCXML_DATAMODEL
public:
    explicit checkerModel(QScxmlStateMachine *parent = nullptr, statusDescriptions *desc = nullptr);

    void checkType(QString barcode);

    void onMainConnected() Q_DECL_OVERRIDE;
    void onUnlocked();

    void onReady();
    void onCheckCovid();
    void onWaitCovidContoller();

    void onShowNoTicket();
    void onShowCovidCheckCommunicationTimeout();
    void onShowCovidFail();

private slots:

private:
    void onLocEnBc();
    void onLocExBc();

};

#endif // CHECKERMODEL_H
