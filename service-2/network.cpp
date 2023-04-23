#include "network.h"

network::network(QSettings *settings)
{
    settings->beginGroup( "network" );
    broadcastPort = settings->value( "broadcastPort", 27006 ).toInt();
    wicketTCP_port = settings->value( "wicketTCP_port", 27005 ).toInt();
    reconnectInterval = settings->value( "reconnectInterval", 2000 ).toInt();

    settings->endGroup();
}

network::~network()
{}

void network::start()
{
    emit logger("start network: " );
    //=============== CLIENT UDP LISTENER =============================================
    main_wicket_search_udpSocket = new QUdpSocket();

    connect(main_wicket_search_udpSocket, &QUdpSocket::readyRead, this, &network::slot_udp_search_readyRead );
    while (!main_wicket_search_udpSocket->bind(QHostAddress::Any, broadcastPort ) )  //слушаю все https://ru.stackoverflow.com/questions/792489/qt-qudpsocket-%D0%BF%D1%80%D0%B8%D0%B5%D0%BC-%D0%B4%D0%B5%D0%B9%D1%82%D0%B0%D0%B3%D1%80%D0%B0%D0%BC%D0%BC%D1%8B-broadcast
    {
        emit logger("Server Error - Unable to start the UDP listener: " + main_wicket_search_udpSocket->errorString());
        main_wicket_search_udpSocket->close();
    }

    //=============== CLIENT TCP LISTENER =============================================
//    main_wicket_ctrl_tcpServer = new QTcpServer();
    main_wicket_ctrl_tcpServer = new QTcpServer();
    main_wicket_ctrl_tcpServer->setMaxPendingConnections(numConnections);


    connect(main_wicket_ctrl_tcpServer, &QTcpServer::newConnection, this, &network::newReaderConnection );
//    connect(main_wicket_ctrl_tcpServer, &TCPServer::incomingDescriptor, this, &network::newReaderConnected2 );



    while (!main_wicket_ctrl_tcpServer->listen(QHostAddress::Any, this->wicketTCP_port))
    {
        emit logger("Server Error - Unable to start the TCP server: " + main_wicket_ctrl_tcpServer->errorString());
        main_wicket_ctrl_tcpServer->close();
    }

}

void network::newReaderConnection()
{
    QTcpSocket* soc = main_wicket_ctrl_tcpServer->nextPendingConnection();

    readerConnector *reader = new readerConnector(soc, 10000);

    emit newReaderConnected(reader);
}

void network::slot_udp_search_readyRead()
{
    QByteArray datagramm;
    quint16 port;
    QHostAddress _ip_addr(0xFFFFFFFF);
    while ( main_wicket_search_udpSocket->hasPendingDatagrams() )
    {
        datagramm.resize(int(main_wicket_search_udpSocket->pendingDatagramSize()));
        main_wicket_search_udpSocket->readDatagram(datagramm.data(), datagramm.size(), &_ip_addr, &port);
            QByteArray a = addrrr.take(_ip_addr);
            a.append(datagramm);
            //qDebug() << "a " << "_ip_addr" << _ip_addr << a;
            if ( ( a.contains("turnstile" )) || ( a.contains("widget_client" )) )
            {
                main_wicket_search_udpSocket->writeDatagram(QByteArray("server_v3"), _ip_addr, port);

            }
            else
                if(a.size() < 0xffff)
                    addrrr.insert(_ip_addr, a);
    }
}
