#ifndef DB_WORKER_H
#define DB_WORKER_H

//#include <wicketFSM/wicket-fsm-state.h>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>
#include <QVariant>

#include <QThread>

#include <QElapsedTimer>

#include "common_types.h"

class db_worker: public QObject {
    Q_OBJECT

public:
    db_worker(QObject *parent = nullptr, gateData *data = nullptr, QString conn_str="");
    ~db_worker();

    QString gate_ptr_to_string_for_db_connection_id;

private:
    QString identity = "КПП: " + data->Caption + ", номер: " + QString::number(data->GateNumber);
    gateData *data;
    //wicketFSM *gate;
    QString GateCaption{};
    QString conn_str;
    QElapsedTimer check_ticket_processing_time;
    QElapsedTimer confirm_pass_processing_time;
    bool db_open();
    QString ticket_state_description(int desc);

public slots:
    void create_db_connect();
    void _register(QString MAC, QString IP);
    void checkCovidControler(QString barcode);
    void checkTicket(int dir, QString message);
    void confirmPass(int dir, QString barcode);

private slots:
    void slot_timeout();

signals:
    void logger( QString );

    void dbFail();

    void covidControlerOK(QString);
    void ticketControllerOK(QString);
    void ticketOK(QString);

    void ticketFail(QString);
    void s_covidControlerFail();

    void count(int);

};

#endif // DB_WORKER_H
