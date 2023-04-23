#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QDataStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include <QTimer>

#include <QSettings>

#include "tcpserver.h"
//#include <common_types.h>
#include "reader-connector.h"

class network : public QObject {
    Q_OBJECT
public:
    network(QSettings* settings = nullptr);
    ~network();

private:
    QHash<int, QTcpSocket*> soc_list;                     // список сокетов связанных с турникетами
    QHash<QHostAddress, QByteArray> addrrr;               // датаграммы принятые от турникета

    int numConnections = 160;
    quint16 broadcastPort  = 27006;
    quint16 wicketTCP_port = 27005;
    int reconnectInterval = 6000;

//    QTcpServer  *main_wicket_ctrl_tcpServer;
    TCPServer  *main_wicket_ctrl_tcpServer;
    QUdpSocket *main_wicket_search_udpSocket;

public slots:
    void start();

private slots:
    void newReaderConnection();
    void slot_udp_search_readyRead();
    void checkMAC();

signals:
    void logger( QString );
    void newReaderConnected2(int);
    void newReaderConnected3(readerConnector*);

};

#endif // NETWORK_H
