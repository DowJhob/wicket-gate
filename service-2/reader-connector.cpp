#include "reader-connector.h"

readerConnector::readerConnector(int socketDescriptor, int reconnectInterval)
{
    socket.setSocketDescriptor(socketDescriptor//,QTcpSocket::SocketState::BoundState, QIODevice::ReadWrite
                                );
    in.setDevice(&socket);
    in.setVersion(QDataStream::Qt_5_12);
    connect(&p_reconnect_timer,  &QTimer::timeout,            &socket,             &QTcpSocket::abort);
    connect(&socket,             &QTcpSocket::disconnected,   &p_reconnect_timer, &QTimer::stop);
    connect(&socket,             &QTcpSocket::disconnected,   this,               &readerConnector::readerDisconnected); //провайдим сигнал дальше (в мэйн)
    connect(&socket,             &QTcpSocket::disconnected,   this,               &readerConnector::deleteLater);
    //connect(socket,             &QTcpSocket::disconnected,   socket,             &QTcpSocket::deleteLater);
    connect(&socket,             &QTcpSocket::readyRead,      this,               &readerConnector::slot_readyRead,    Qt::DirectConnection );
    p_reconnect_timer.setInterval(reconnectInterval);
    p_reconnect_timer.setSingleShot(true);
    p_reconnect_timer.start();
    addr = QHostAddress(socket.peerAddress().toIPv4Address()).toString();
//    connect(this,   &readerConnector::getMAC,     this,  [this](){
//        send_to_socket(message( MachineState::undef, command::getMAC));} );

}

readerConnector::~readerConnector()
{
    //   socket->deleteLater();
    //     p_reconnect_timer->deleteLater();

}

void readerConnector::requestMAC()
{
    send_to_socket(message( MachineState::undef, command::getMAC));
}

void readerConnector::send_to_socket(message message)
{
//    qDebug() << "==================== send_to_socket " << addr << this->socket;
    //if(message.cmd != command::heartbeat)
    //    qDebug() << (int)message.cmd;
    if ( !socket.isValid() || socket.state() != QAbstractSocket::ConnectedState )
        return;
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << quint16(0) << message;
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));
    socket.write(arrBlock);
}

void readerConnector::slot_readyRead()
{
    //qDebug() <<  " ========= время между посылками: GateNum:   " <<GateNum << parse_time.nsecsElapsed()/1000000 << "ms";
    //qDebug() <<  " ====";
    p_reconnect_timer.start();
    send_to_socket( message()); // в ответ жарим что ни будь, что бы там не произошел реконнект по умолчанию command(command_type::command, "heartbeat")
    message str{};
    for ( uint i = 0; i < 0xFFFF; i++)
    {
        if (!m_nNextBlockSize) {
            if (socket.bytesAvailable() < sizeof(quint16))
                break;
            in >> m_nNextBlockSize;
        }
        if (socket.bytesAvailable() < m_nNextBlockSize)
            break;
        in >> str;
        m_nNextBlockSize = 0;
        if(str.cmd == command::onRegister)
            emit recieveMAC(this, str.body.toString());
        //else if(str.cmd == command::getTemp)
        //    emit getTemp(str.body.toString());
        else
            emit read( str );
    }
}
