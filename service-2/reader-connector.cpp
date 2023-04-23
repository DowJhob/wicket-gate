#include "reader-connector.h"

readerConnector::readerConnector(int socketDescriptor, int reconnectInterval):socketDescriptor(socketDescriptor),reconnectInterval(reconnectInterval)
{
    readerThread = new QThread();
    connect(readerThread,  &QThread::started,            this,             &readerConnector::start);
    connect(readerThread,  &QThread::finished,   readerThread,             &QThread::deleteLater);
    moveToThread(readerThread);
    readerThread->start();


}

readerConnector::readerConnector(QTcpSocket* pSocket, int reconnectInterval):pSocket(pSocket),reconnectInterval(reconnectInterval)
{
    readerThread = new QThread();
    connect(readerThread,  &QThread::started,            this,             &readerConnector::start2);
    connect(readerThread,  &QThread::finished,   readerThread,             &QThread::deleteLater);
    moveToThread(readerThread);
    readerThread->start();


}

void readerConnector::start2()
{
    p_reconnect_timer = new QTimer();

    pSocket->moveToThread(readerThread);

    in.setDevice(pSocket);
    in.setVersion(QDataStream::Qt_5_12);
    connect(p_reconnect_timer, &QTimer::timeout,           pSocket,            &QTcpSocket::abort,    Qt::DirectConnection );

    connect(p_reconnect_timer, &QTimer::timeout,           this,              [this](){qDebug() << "QTimer::timeout" << addr;});

    connect(pSocket,            &QTcpSocket::disconnected,  p_reconnect_timer, &QTimer::stop,    Qt::DirectConnection );
    connect(pSocket,            &QTcpSocket::disconnected,  this,              &readerConnector::readerDisconnected); //провайдим сигнал дальше (в мэйн)
    connect(pSocket,            &QTcpSocket::disconnected,  this,              &readerConnector::deleteLater);
    //connect(socket,             &QTcpSocket::disconnected,   socket,             &QTcpSocket::deleteLater);
    connect(pSocket,            &QTcpSocket::readyRead,     this,              &readerConnector::slot_readyRead,    Qt::DirectConnection );

    connect(pSocket,            &QTcpSocket::disconnected,  this,              [this](){qDebug() << "QTcpSocket::disconnected" << addr;});
    connect(pSocket,            &QTcpSocket::errorOccurred, this,              [this](QAbstractSocket::SocketError err){qDebug() << "QTcpSocket::errorOccurred" << addr << err;});
    //    connect(socket,            SIGNAL(error()),         this,          SLOT(test())    );
    connect(pSocket,            &QTcpSocket::stateChanged,  this,              [this](QAbstractSocket::SocketState state){qDebug() << "QTcpSocket::stateChanged" << addr << state;});

    //    connect(socket,            &QTcpSocket::,  this,              [this](QAbstractSocket::SocketState state){qDebug() << "QTcpSocket::stateChanged" << addr << state;});


    p_reconnect_timer->setInterval(reconnectInterval);
    p_reconnect_timer->setSingleShot(true);
    p_reconnect_timer->start();
    addr = QHostAddress(pSocket->peerAddress().toIPv4Address()).toString();
    //    connect(this,   &readerConnector::getMAC,     this,  [this](){
    //        send_to_socket(message( MachineState::undef, command::getMAC));} );
}

void readerConnector::start()
{
    p_reconnect_timer = new QTimer();
    pSocket = new QTcpSocket();

    pSocket->setSocketDescriptor(socketDescriptor//,QTcpSocket::SocketState::BoundState, QIODevice::ReadWrite
                                );
    in.setDevice(pSocket);
    in.setVersion(QDataStream::Qt_5_12);
    connect(p_reconnect_timer, &QTimer::timeout,           pSocket,            &QTcpSocket::abort,    Qt::DirectConnection );

    connect(p_reconnect_timer, &QTimer::timeout,           this,              [this](){qDebug() << "QTimer::timeout" << addr;});

    connect(pSocket,            &QTcpSocket::disconnected,  p_reconnect_timer, &QTimer::stop,    Qt::DirectConnection );
    connect(pSocket,            &QTcpSocket::disconnected,  this,              &readerConnector::readerDisconnected); //провайдим сигнал дальше (в мэйн)
    connect(pSocket,            &QTcpSocket::disconnected,  this,              &readerConnector::deleteLater);
    //connect(socket,             &QTcpSocket::disconnected,   socket,             &QTcpSocket::deleteLater);
    connect(pSocket,            &QTcpSocket::readyRead,     this,              &readerConnector::slot_readyRead,    Qt::DirectConnection );

    connect(pSocket,            &QTcpSocket::disconnected,  this,              [this](){qDebug() << "QTcpSocket::disconnected" << addr;});
    connect(pSocket,            &QTcpSocket::errorOccurred, this,              [this](QAbstractSocket::SocketError err){qDebug() << "QTcpSocket::errorOccurred" << addr << err;});
    //    connect(socket,            SIGNAL(error()),         this,          SLOT(test())    );
    connect(pSocket,            &QTcpSocket::stateChanged,  this,              [this](QAbstractSocket::SocketState state){qDebug() << "QTcpSocket::stateChanged" << addr << state;});

    //    connect(socket,            &QTcpSocket::,  this,              [this](QAbstractSocket::SocketState state){qDebug() << "QTcpSocket::stateChanged" << addr << state;});


    p_reconnect_timer->setInterval(reconnectInterval);
    p_reconnect_timer->setSingleShot(true);
    p_reconnect_timer->start();
    addr = QHostAddress(pSocket->peerAddress().toIPv4Address()).toString();
    //    connect(this,   &readerConnector::getMAC,     this,  [this](){
    //        send_to_socket(message( MachineState::undef, command::getMAC));} );
}

//void readerConnector::test(QAbstractSocket::SocketError err)
//{
//   qDebug() << "QTcpSocket::error" << addr << err;

//}

readerConnector::~readerConnector()
{
    pSocket->deleteLater();
    p_reconnect_timer->deleteLater();
    readerThread->exit();
    qDebug() << "readerConnector::~readerConnector" << addr;
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
    if ( !pSocket->isValid() || pSocket->state() != QAbstractSocket::ConnectedState )
        return;
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << quint16(0) << message;
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));
    pSocket->write(arrBlock);
}

void readerConnector::slot_readyRead()
{
    //qDebug() <<  " ========= время между посылками: GateNum:   " <<GateNum << parse_time.nsecsElapsed()/1000000 << "ms";
    //qDebug() <<  " ====";
    p_reconnect_timer->start();
    send_to_socket( message()); // в ответ жарим что ни будь, что бы там не произошел реконнект по умолчанию command(command_type::command, "heartbeat")
    message str{};
    for ( uint i = 0; i < 0xFFFF; i++)
    {
        if (!m_nNextBlockSize) {
            if (pSocket->bytesAvailable() < sizeof(quint16))
                break;
            in >> m_nNextBlockSize;
        }
        if (pSocket->bytesAvailable() < m_nNextBlockSize)
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
