#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <QString>
#include <QTcpSocket>


struct gateData
{
    QString Caption;
    int GateNumber;
    QString MainMAC;
    QString SlaveMAC;
    QString EntryMAC;
    QString ExitMAC;
    QString MainIP;
    QString SlaveIP;
    bool standAlone;
    bool certCheck;
};

typedef struct gateState
{
    gateState(QString state, QString body=""):state(state), body(body)
    {

    }
    QString state;
    QString body;
}gateState;

enum dir_type {
    entry_ = 1,
    exit_ = -1
};

struct statusDescriptions
{
    QString MainNotReady = "Ведущий считыватель не готов";
    QString PlaceCovidQR = "Предъявите сертификат";
    QString certFirst = "Сначала предъявите сертификат. Билет на мероприятие допущен.";
    QString certCtrlFirst = "Сначала сертификатный контролер должен проверить паспорт. Билет на мероприятие допущен.";
    QString checkCovidQR = "Проверяем сертификат";
    QString CovidNoTicket = "Сертификат не распознан";
    QString CovidQRFail = "Сертификат просрочен, заблокирован или невалиден";
    QString gosUslugiFail = "Нет связи с сервером Госуслуг";

    QString PlaceCovidControlerQR = "Предъявите QR контролера";
    QString CovidControlerQRCheck = "Проверяем контролера";
    QString CovidControlerQR_Fail = "Контролер не опознан";
    QString CertCtrlFail = "Сертификатный контролер не может осуществлять пропуск";
    QString DbFail = "База данных не отвечает";

    QString PlaceTicket = "Предъявите билет";
    QString checkTicket = "Проверяем билет";


    QString checkDoubleScan = "Проверяем двойной проход";
    QString doubleScanFail = "Попытка двойного прохода ";
    QString pleaseComeThrough = "Пожалуйста проходите";
    QString pleaseWaitCounterWay = "Пожалуйста подождите, вам идут навстречу";
    QString pleaseWaitSecurityCheck = "Пожалуйста подождите, идет досмотр предыдущего посетителя";


    QString TicketFail = "";
    QString TicketOK = "";
};

#endif // COMMON_TYPES_H
