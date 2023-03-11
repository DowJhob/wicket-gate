#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDateTime>
#include <QCoreApplication>
#include <QDebug>
#include <QTextCodec>

class Logger: public QObject {
    Q_OBJECT
public:
    Logger();
public slots:
    void logger( QString str );
private:
    QDate startDate;
    QString path = QCoreApplication::applicationDirPath();
    QFile log_file;
    QTextCodec *codec = QTextCodec::codecForName("CP866");
    void newFile(  );
    //QScopedPointer<QFile>   m_logFile;

    // Объявляение обработчика
    //void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    //void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    //{
    //    // Открываем поток записи в файл
    //    QTextStream out(m_logFile.data());
    //    // Записываем дату записи
    //    out << QDateTime::currentDateTime().toString("hh:mm:ss.zzz / yyyy-MM-dd");
    //    // По типу определяем, к какому уровню относится сообщение
    //    switch (type)
    //    {
    //    case QtInfoMsg:     out << "INF "; break;
    //    case QtDebugMsg:    out << "DBG "; break;
    //    case QtWarningMsg:  out << "WRN "; break;
    //    case QtCriticalMsg: out << "CRT "; break;
    //    case QtFatalMsg:    out << "FTL "; break;
    //    }
    //    // Записываем в вывод категорию сообщения и само сообщение
    //    out << context.category << ": " << msg << endl;
    //    out.flush();    // Очищаем буферизированные данные

    //    qDebug().noquote()
    //            <<   msg ;
    //}

};

#endif // LOGGER_H
