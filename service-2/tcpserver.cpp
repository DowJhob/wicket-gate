#include "tcpserver.h"

TCPServer::TCPServer(QObject *parent)
    : QTcpServer{parent}
{

}

void TCPServer::incomingConnection(int socketDescriptor)
{
    emit incomingDescriptor(socketDescriptor);
}
