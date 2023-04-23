#include "reader-connector.h"

readerConnector::readerConnector(QTcpSocket* pSocket, int reconnectInterval):pSocket(pSocket),reconnectInterval(reconnectInterval)
{
    readerThread = new QThread();
    connect(readerThread,  &QThread::started,            this,             &readerConnector::start);
    connect(readerThread,  &QThread::finished,   readerThread,             &QThread::deleteLater);
    moveToThread(readerThread);
    pSocket->setParent(0);
    pSocket->moveToThread(readerThread);
    readerThread->start();
}

void readerConnector::start()
{
    p_reconnect_timer = new QTimer();
    in.setDevice(pSocket);
    in.setVersion(QDataStream::Qt_5_12);
    connect(p_reconnect_timer, &QTimer::timeout,           pSocket,           &QTcpSocket::abort,                   Qt::DirectConnection );

    connect(pSocket,           &QTcpSocket::disconnected,  p_reconnect_timer, &QTimer::stop,                        Qt::DirectConnection );
    connect(pSocket,           &QTcpSocket::disconnected,  this,              &readerConnector::readerDisconnected, Qt::DirectConnection); //провайдим сигнал дальше (в мэйн)
    connect(pSocket,           &QTcpSocket::disconnected,  this,              &readerConnector::deleteLater,        Qt::DirectConnection);

    connect(pSocket,           &QTcpSocket::readyRead,     this,              &readerConnector::slot_readyRead,     Qt::DirectConnection );

    addr = QHostAddress(pSocket->peerAddress().toIPv4Address()).toString();
    p_reconnect_timer->setInterval(reconnectInterval);
    p_reconnect_timer->setSingleShot(true);
    p_reconnect_timer->start();
}

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
