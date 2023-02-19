#include <QCoreApplication>
#include <windows.h>
#include <QObject>
#include <QFile>
#include <QThread>
#include <QtCore>

#include <common_types.h>
#include <main_server.h>
#include "logger.h"
#include "network.h"
#include "ssl_websocket_server.h"

#include "http_server.h"

#include "covidcertchecker.h"

#define TCP_CLIENT_CONTROL_PORT 27005
#define UDP_CLIENT_SEARCH_PORT 27006

#define TCP_FILE_TRANSFER_PORT 27007

#define TCP_WIDGET_CONTROL_PORT 27008
#define UDP_WIDGET_SEARCH_PORT 27009

static HWND GetConsoleHwnd(void)
{
#define MY_BUFSIZE 1024
    HWND hwndFound;
    char pszNewWindowTitle[MY_BUFSIZE]{'P','r','o','m','e','t','e','u','s'};
    SetConsoleTitleA(pszNewWindowTitle);
    Sleep(40);
    hwndFound=FindWindowA(NULL, pszNewWindowTitle);
    return(hwndFound);
}

static void setwnd(void)
{
    HANDLE hConsole_c = GetStdHandle(STD_OUTPUT_HANDLE);
    // извлекает размер самого большого возможного консольного окна, основанного на текущем шрифте и размере дисплея, в символах.
    COORD winSize = GetLargestConsoleWindowSize(hConsole_c);
    SHORT dx = winSize.X/10;
    SHORT dy = winSize.Y/10;
    COORD buf_size{static_cast<SHORT>(winSize.X-2*dx), static_cast<SHORT>(winSize.Y-2*dy)};  // минус 20%
    SetConsoleScreenBufferSize( hConsole_c, buf_size );
    SMALL_RECT src = {0,0, static_cast<SHORT>(buf_size.X-1), static_cast<SHORT>(buf_size.Y-1)};
    // устанавливает текущий размер и позицию окна экранного буфера консоли.
    SetConsoleWindowInfo ( hConsole_c, TRUE, &src);

    WINDOWINFO pwi;
    HWND hwnd = GetConsoleHwnd();
    qDebug() <<  GetWindowInfo( hwnd, &pwi ) << pwi.rcWindow.right << pwi.rcWindow.bottom;

    long width = pwi.rcWindow.right - pwi.rcWindow.left;
    long height = pwi.rcWindow.bottom - pwi.rcWindow.top;


    MoveWindow(hwnd, 1, 1, width, height, TRUE);
}

int main(int argc, char *argv[])
{
    //setwnd();
    QCoreApplication a(argc, argv);
//    a.setApplicationName(QString("CAS Prometeus by eulle@ya.ru ver %1").arg(GIT_VERSION));
//    qDebug() << QString("CAS Prometeus by eulle@ya.ru ver %1").arg(GIT_VERSION);

    qRegisterMetaType<message>("message");
    //    qRegisterMetaType<gateState>("gateState");
    QString path = QCoreApplication::applicationDirPath() + "/settings.ini";
    qDebug() << "File settings path :" << path;
    if( !QFile::exists(path) )
    {
        //       emit logger("File settings not found! Default settings!");
        qDebug() << "File settings not found! Default settings! :";
        //      return def ;
    }
    QSettings settings( path, QSettings::IniFormat );

    QThread networkThread;
    QThread main_serverThread;
    QThread httpThread;

    Logger Log;
    main_server   *_main_server = new main_server( nullptr, &settings );
    network *net = new network( &settings );
    http_server http(settings);

    _main_server->moveToThread(&main_serverThread);
    net->moveToThread(&networkThread);
    http.moveToThread(&httpThread);

    QObject::connect(&main_serverThread, &QThread::started,              _main_server, &main_server::start);
    QObject::connect(&main_serverThread, &QThread::started,              net,          &network::start);
    QObject::connect(&httpThread,        &QThread::started,              &http,        &http_server::start);

    QObject::connect(_main_server,       &main_server::logger,           &Log,         &Logger::logger );
    QObject::connect(net,                &network::logger,               &Log,         &Logger::logger );
    QObject::connect(&http,              &http_server::logger,           &Log,         &Logger::logger );
    ///===================================================================================================================================================================

    QObject::connect(_main_server,       &main_server::send_to_widgets2,  &http,       &http_server::send_to_widgets );
    ///===================================================================================================================================================================
    QObject::connect(net,                &network::newReaderConnected2,  _main_server, &main_server::newReaderConnected2 );
    QObject::connect(&http,              &http_server::widget_readyRead, _main_server, &main_server::slot_widget_readyRead );

    main_serverThread.start(QThread::HighestPriority);
    networkThread.start(QThread::HighestPriority);

    httpThread.start();

    a.exec();
    return 0;
}
