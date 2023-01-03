#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QObject>

#include <QJsonObject>
#include <QJsonDocument>

#include <QDebug>
#include <QTimer>

#include <QThread>
#include <QElapsedTimer>
#include <QSettings>
#include <QCoreApplication>
#include <QFile>
#include <QHostInfo>

#include "QtWebSockets/QWebSocket"
#include "reader-connector.h"
//#include <logical_reader.h>

#define UNLOCK_QR "Открыть Все"

#include "wicketgate.h"

class main_server : public QObject {
    Q_OBJECT
public:
    main_server( QObject* parent = nullptr, QSettings* settings = nullptr);
    ~main_server();

public slots:
    void start();

    void newReaderConnected2(int desc);

    void registerWicketReaderByMAC(readerConnector *s, QString MAC);

    /// \brief slot_widget_readyRead принимаем список команд разделенный ;
    /// каждая команда состоит из полей разделенных :
    /// если поле одно это команда всем турникетам
    /// второе поле содержит ip адрес одного турникета
    /// если полей три (первые два пустые) команда серверу, например отключить проверку двойного прохода
    /// в дальнейшем нужно изменить на префиксы
    void slot_widget_readyRead(QString str);

    void ticketCounter(int dir);

private slots:
    void checkDoubleScan(QString barcode);

    void m_resetDoubleScan(QString barcode);

private:
    QHash<QString, wicketGate*> GateByMAC;             // Все маки из базы
    QHash<QString, wicketGate*> GateByCaption;             // гейты по имени группы и номеру, что бы управлять из виджета
    QHash<QString, wicketGate*> wicketGateListByBarcode{}; // По ШК кладем турникеты

    ///
    /// \brief doubleScanSwitcher При значении false выключает проверку двойного прохода
    ///
    bool doubleScanSwitcher = true;
    QString unlockCard;
    QString armedCard;
    QSqlDatabase db;
    QElapsedTimer t;
    QSettings* settings;
    QString conn_str;

    /// \brief ticket_pass_data Контейнер с количествами проходов на каждое мп.
    ///  в паре first количество в зале, second - количество всего посетивших
    QMap<QString, QPair<int, int>> ticket_pass_data;

    int total_count = 0;
    int current_count = 0;

    void getSettings(QSettings* settings = nullptr);

    void create_db_connect(QString conn_str);
    bool db_open();
    void get_Gate_data_from_DB();
    int register_gate_in_db(QString MAC, QString IP);

    void doubleScanOff();
    void doubleScanOn();
    void doubleScanClear();

    void get_ticket_pass_count();


    void sendTelemetry(QJsonObject json);


    typedef void (*PTR_FUN)(wicketGate *gate);
    QHash<QString, PTR_FUN> ccccc{
//        {"",           [](wicketGate *gate){}},
//        {"",           [](wicketGate *gate){}},
        {"unlock",     [](wicketGate *gate){gate->setUnLocked();}},
        {"armed",      [](wicketGate *gate){gate->setArmed();}},
        {"stndAlnOn",  [](wicketGate *gate){gate->stndAlnOn();}},
        {"stndAlnOff", [](wicketGate *gate){gate->stndAlnOff();}},
        {"certOn",     [](wicketGate *gate){gate->certOn();}},
        {"certOff",    [](wicketGate *gate){gate->certOff();}},
        {"sync",       [](wicketGate *gate){gate->monitor->synchronizeMonitor();}},
        {"dump",       [](wicketGate *gate){qDebug() << gate->monitor->dumpAllActiveStates();}}
    };
    PTR_FUN pTest;

signals:
    void requestMAC();
    void logger( QString );
    void send_to_widgets2(QString );
    //======================== сигналы мэйна подключенные нетворку ==============================================
    void send_to_all_wickets(message );

};

#endif // TCP_SERVER_H


