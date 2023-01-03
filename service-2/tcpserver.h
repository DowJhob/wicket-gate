#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QObject>

class TCPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TCPServer(QObject *parent = nullptr);

    void incomingConnection(int socketDescriptor);

signals:
    void incomingDescriptor(int socketDescriptor);

};

#endif // TCPSERVER_H
