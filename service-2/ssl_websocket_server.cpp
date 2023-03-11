#include "ssl_websocket_server.h"

SslWebSocketServer::SslWebSocketServer(QSettings *settings, HttpSessionStore *sessionStore, QObject *parent) : QObject(parent), m_pWebSocketServer(nullptr), sessionStore(sessionStore)
{
    quint16 webSocketPort;
    QString sslKeyFile;
    QString sslCertFile;
    settings->beginGroup( "network" );
    sslKeyFile = settings->value( "sslKeyFile", "/localhost.key" ).toString();
    sslCertFile = settings->value( "sslCertFile", "/localhost.cert" ).toString();
    webSocketPort = settings->value( "webSocketPort", 27010 ).toInt();
    bool secure_mode = settings->value( "webSocket_secure_mode", false ).toBool();
    //        if(!secure_mode)
    //        {
    //            QFile f("/docroot/index.html");
    //            if(f.open(QIODevice::ReadWrite))
    //            {
    //                QByteArray a = f.readAll();
    //                a.replace("'wss://'", "'ws://'");
    //                f.write(a);
    //            }
    //            else

    //        }
    settings->endGroup();

    QString supp = (QSslSocket::supportsSsl()?"":"not found");
    qDebug() << ("SSL lib " + supp + "ver: " + QSslSocket::sslLibraryBuildVersionString() + QSslSocket::sslLibraryVersionString());
    //     qDebug() <<  QSslSocket::sslLibraryBuildVersionString() << QSslSocket::sslLibraryVersionString();

    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("SSL Echo Server"),(secure_mode)?QWebSocketServer::SecureMode:QWebSocketServer::NonSecureMode, this);
    QSslConfiguration sslConfiguration;
    QFile certFile( QCoreApplication::applicationDirPath() + "/" + sslCertFile );
    QFile keyFile( QCoreApplication::applicationDirPath() + "/" + sslKeyFile );
    qDebug() << "SslWebSocketServer: cert file open:" << certFile.open(QIODevice::ReadOnly);
    qDebug() << "SslWebSocketServer: key file open:" << keyFile.open(QIODevice::ReadOnly);
    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    //     sslConfiguration.
    sslConfiguration.setProtocol(QSsl::TlsV1SslV3);
    m_pWebSocketServer->setSslConfiguration(sslConfiguration);

    if (m_pWebSocketServer->listen(QHostAddress::Any, webSocketPort))
    {
        qDebug() << "SSL WEBsocket Server listening on port" << webSocketPort;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &SslWebSocketServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::sslErrors,     this, &SslWebSocketServer::onSslErrors);
    }
}

SslWebSocketServer::~SslWebSocketServer()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void SslWebSocketServer::send_to_socket(QWebSocket *pSocket, const QString &message)
{
    pSocket->sendTextMessage(message);
}

void SslWebSocketServer::send_to_widgets(QString message)
{
    for( QWebSocket *pSocket : qAsConst(m_clients))
        pSocket->sendTextMessage(message);
}

void SslWebSocketServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    QList<QNetworkCookie> cookiel =
            pSocket->request().header(QNetworkRequest::CookieHeader).value<QList<QNetworkCookie>>();
    QByteArray sessionid;
    for ( const QNetworkCookie &qq: qAsConst(cookiel) )
        if (qq.name() == "sessionid")
        {
            sessionid = qq.value();
            break;
        }
    if ( sessionStore->getSession(sessionid).isNull() )
    {
        pSocket->close(QWebSocketProtocol::CloseCodePolicyViolated, "non autorize!");
        emit logger("non autorize WebSoc client disconnected:" + pSocket->peerName() + pSocket->origin());
        return;
    }

    emit logger("WebSoc client connected:" + pSocket->peerName() + pSocket->origin());
    connect(pSocket, &QWebSocket::textMessageReceived,   this, &SslWebSocketServer::widget_readyRead);
    connect(pSocket, &QWebSocket::disconnected,          this, &SslWebSocketServer::socketDisconnected);
    m_clients << pSocket;

    qDebug() << "sessionid : " << sessionid;
}

void SslWebSocketServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    emit logger( "websoc widget Client disconnected:" + pClient->peerName() + pClient->origin());
    if (pClient)
    {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void SslWebSocketServer::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "WebSoc Ssl errors occurred" << errors;
}
