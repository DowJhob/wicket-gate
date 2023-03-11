#include "logger.h"

Logger::Logger()
{
    QString filename= startDate.toString("'log_'dd_MM_yyyy'.txt'");
    newFile();

    //    m_logFile.reset(new QFile(QCoreApplication::applicationDirPath() + "/" + "logFile.txt"));
    // Открываем файл логирования
    //    m_logFile.data()->open(QFile::Append | QFile::Text);
    // Устанавливаем обработчик
    //    qInstallMessageHandler(messageHandler);

    //qSetMessagePattern("%{type} %{if-category}%{category}: %{endif}%{function}: %{message}");


}



void Logger::logger(QString str)
{
    if ( startDate != QDate::currentDate() )
        newFile();
    str = QTime::currentTime().toString() + " | " + str;
    log_file.write(str.toUtf8() + "\n");
    qDebug().noquote() << QString::fromLocal8Bit( codec->fromUnicode( str ) );
    log_file.flush();
}



void Logger::newFile()
{
    log_file.close();
    startDate = QDate::currentDate();
    log_file.setFileName( path + "/" + startDate.toString("'log_'dd_MM_yyyy'.txt'") );
    if ( !log_file.open(QIODevice::Append) )
        qDebug().noquote() << QString::fromLocal8Bit( codec->fromUnicode( "Лог файл не открыт!\n" ) );
}
