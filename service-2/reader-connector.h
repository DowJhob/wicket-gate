#ifndef READER_CONNECTOR_H
#define READER_CONNECTOR_H

#include <QObject>
#include <QTcpSocket>

#include <QDataStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QTimer>
#include <QHostAddress>
#include <QThread.h>

#include "command.h"

// РџРѕРґРґРµСЂР¶РёРІР°РµРј РєРѕРЅРЅРµРєС‚ РїРѕ С‚Р°Р№РјРµСЂСѓ
class readerConnector:public QObject
{
    Q_OBJECT
public:
    QString addr;
    readerConnector(QTcpSocket* pSocket, int reconnectInterval=20000);
    ~readerConnector();

public slots:
    void requestMAC();

    void send_to_socket( message message);

private slots:
    void slot_readyRead();
    void start2();

//    void test(QAbstractSocket::SocketError err);

private:
    QTcpSocket* pSocket;
    QThread *readerThread;
    QTimer *p_reconnect_timer;
    QDataStream in;
    quint16 m_nNextBlockSize = 0;
    int socketDescriptor;
    int reconnectInterval;

signals:
    void recieveMAC(readerConnector*, QString);
    void read(message);
    void readerDisconnected();
};

#endif // READER_CONNECTOR_H
