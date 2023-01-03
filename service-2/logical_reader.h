#ifndef WICKET_READER_H
#define WICKET_READER_H

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
//#include <QHostAddress>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QDataStream>
#include <QTime>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include <common_types.h>
#include "reader-connector.h"


class  db_worker;
class logical_reader : public QObject{
    Q_OBJECT
public:
    QJsonObject ja{
        {"heartbeat", ""},
        {"GateCaption", ""},
        {"GateNum", ""},
        {"rdr_type", -1},
        {"IP","255.255.255.255"},
        {"temp",""},
        {"wicket_state", 0},
        {"barcode",""},
        {"total_count",0},
        {"current_count",0}
    };
    QJsonDocument gateCaptionJSON;

    QString MAC{};
    QString addr;
    QString barcode{};
    QString GateCaption{};
    int GateNum = 0;

    dir_type direction_type = dir_type::entry_;
    _reader_type reader_type = _reader_type::_main;



    QString string_direction = "undef";
    logical_reader* linkedReader = nullptr;

    int current_dir = 0;

    socket_wrapper *sock_wrap;
    db_worker *_db_worker = nullptr;
    QString conn_str;
    QElapsedTimer barcode_handle_timer;

    logical_reader(socket_wrapper* sock_wrap = nullptr, QString conn_str = {}, QObject* p_main_srv = nullptr) : QObject(p_main_srv), sock_wrap(sock_wrap), conn_str(conn_str)
    {
        gateCaptionJSON.setObject(ja);
        create_DB_worker();
        connect(sock_wrap, &socket_wrapper::readerDisconnected,  this,      &logical_reader::slot_wicket_disconnect);
        connect(sock_wrap, &socket_wrapper::read,          this,      &logical_reader::parse );
        connect(this,      &logical_reader::send_to_socket, sock_wrap, &socket_wrapper::send_to_socket);
        addr = sock_wrap->addr;
    }
    ~logical_reader()
    {

    }
    void check_ticket()
    {
        emit request_check_ticket_SIG( "" );
    }

public slots:
    void set_param_from_db(QString GateCaption, int num, QString direction, _reader_type type_v2, dir_type direction_2)
    {
        GateNum = num;

        reader_type = type_v2;
        direction_type = direction_2;
        switch (reader_type)
        {
        case _reader_type::_main :
            emit send_to_socket(message(MachineState::undef, command::set_type_main)); break;
        case _reader_type::slave :
            emit send_to_socket(message(MachineState::undef, command::set_type_slave)); break;//скажем ему что он подчиненный и наверное передадим параметры
        }
        switch (direction_2)
        {
        case dir_type::entry_ : emit send_to_socket(message(MachineState::undef, command::set_type_entry)); break;
        case dir_type::exit_  : emit send_to_socket(message(MachineState::undef, command::set_type_exit)); break;
        }


        string_direction = direction;

        this->GateCaption = GateCaption + ":" + QString::number(num);

        ja["GateCaption"] = GateCaption;
        ja["GateNum"]     = num;
        ja["rdr_type"] = type_v2;
        ja["IP"] = addr;
    }
    void slot_wicket_disconnect()
    {
        emit disconnect(addr);//Первым делом сообщим мэйну что бы он нас везде выпилил
        emit logger(GateCaption + "\t | disconnect \t\t\t\t\t | ip: " + addr );
        ja["wicket_state"] = state::disconnected;
        emit send_to_widgets( ja );
        safe_delete_db_worker();//А тут собственно остановим тред воркера и он сам нас выпилит сигналом
    }

    void telemetry_parse(message message)
    {
        switch (message.state) {
        //case MachineState::heartbeat:      ja["heartbeat"] = str; break;  // по факту логи тут буду слать
        case MachineState::onArmed    : ja["wicket_state"]=state::armed;                   break;
        case MachineState::onUnlocked : ja["wicket_state"]=state::unlocked;                break;
        case MachineState::onReady    : ja["barcode"]=""; ja["wicket_state"]=state::ready; break;
        case MachineState::onStateMachineNotReady : ;break;
        default                                   : break;
        }
        //==============================================  Телеметрия  =====================================================================================
        if ( reader_type == _reader_type::_main ||            // Только главный шлет все
             message.cmd == command::getTemp //||               // подчиненный только температуру
             //   message.state == MachineState::onArmed ||      //
             //   message.state == MachineState::onReady ||       // и статусы что бы в реальности видеть
             //   message.state == MachineState::onUnlocked       // состояние его машины
             )
            emit send_to_widgets( ja );
    }

    void parse(message msg)
    {
        QString str = msg.body.toString();
        QString state;

        if (msg.cmd != command::undef)
            switch (msg.cmd) {
            case command::get_Register : register_(str);   break;
            case command::getTemp      : ja["temp"] = str; break;
            default                    :                 ; break;
            }

        if (msg.state != MachineState::undef)
            switch (msg.state) {
            case MachineState::onCheckEntry  : recieve_barcode(str, dir_type::entry_); break;
            case MachineState::onCheckExit   : recieve_barcode(str, dir_type::exit_);  break;
            case MachineState::onEntryPassed  : confirm_pass(dir_type::entry_);         break;
            case MachineState::onExitPassed  : confirm_pass(dir_type::exit_);          break;

            case MachineState::onPassDropped : pass_drop();                            break;
                //case MachineState::onPassDropped :
                //case MachineState::onWrong       :
            case MachineState::on_dbTimeout  :
                emit remove_bc_from_check(barcode);
                send_to_linked2(msg); break;
            default                          : send_to_linked2(msg); break; //остальное проксируем
            }
        telemetry_parse(msg);

    }

    void slot_check_ticket_result(int check_result)
    {
        state s;
        command cmd;
        QString description = ticket_state_description( check_result );
        if(check_result == 0x0000 ||                  //Вход
                check_result == 0x0100 ||             //Выход
                check_result == 0x1000 )              //Сервис
        {
            if(current_dir == 1)//открываем на вход
            {
                s = state::open_entry;
                cmd = command::setEntryOpen;
            }
            if(current_dir == -1)
            {
                s = state::open_exit;
                cmd = command::setExitOpen;
            }
        }
        else {
            emit remove_bc_from_check(barcode);
            s = state::wrong;
            //cmd = command::set_Wrong;
        }
        emit send_to_socket( message(MachineState::undef, cmd, description));
        ja["wicket_state"] = s;
        QString re_time = " обработка " + QString::number(barcode_handle_timer.nsecsElapsed()/1000000) + "ms";
        emit logger( GateCaption + "\t | " + description + "\t\t\t\t | ip: " + addr + " | barcode: " + barcode + re_time);
        ja["barcode"] = barcode + description + re_time;
        emit send_to_widgets( ja ); //отправим во все подключенные виджеты
    }
    void set_wrong(QString description)
    {
        ja["wicket_state"] = state::wrong;
        ja["barcode"] = barcode + description;
        //barcode.clear();
        emit send_to_socket( message(MachineState::undef, command::set_Wrong, description));
    }

    void send_to_linked2(message msg)
    {
        if ( linkedReader != nullptr )
            emit send_to_linked_socket(msg);
        //linkedReader->send_to_socket( message(msg_type::command, cmd) );
    }

private slots:
    void reconnecting_timer_slot()
    {
        emit logger(GateCaption + "\t | network::reconnecting_timer_slot: " + addr);
        //     sock_wrap->abort();
    }

private:
    void create_DB_worker();
    void safe_delete_db_worker();

    void register_( QString str)
    {
        MAC = str;
        emit  get_register_from_wicket(this);
    }
    void recieve_barcode(QString str, dir_type dir)
    {
        barcode_handle_timer.start();
        current_dir = dir;

        if (direction_type == dir) // Это срабатывает всегда в мэйне, если направление прохода совпадает значит это запрос от мэйна
            send_to_linked2(message(MachineState::getRemoteBarcode, command::undef)); // иначе подчиненный не переключится на проверку и не отработает статусы


        ja["barcode"] = str;
        barcode = str;
        emit get_barcode_from_wicket(this); //отправляем его в мэйн для проверки двойного прохода
    }

    void confirm_pass(dir_type direction)
    {
        QString desc = barcode;
        MachineState st = MachineState::undef;
        if (direction == dir_type::entry_)
        {
            st = MachineState::onEntryPassed;
            ja["wicket_state"]= state::entry_passed;
            desc += " Вход совершен";
        }
        else if (direction == dir_type::exit_)
        {
            st = MachineState::onExitPassed;
            ja["wicket_state"]= state::exit_passed;
            desc += " Выход совершен";
        }
        ja["barcode"] = desc;
        emit remove_bc_from_check(barcode);
        emit request_confirm_pass_SIG( direction );
        emit logger(GateCaption + "\t | " + desc );
        send_to_linked2(message(st, command::undef)); // тут проксируем
    }
    void pass_drop()
    {
        emit remove_bc_from_check(barcode);
        ja["wicket_state"]= state::pass_drop;
        ja["barcode"]= barcode + " Проход не состоялся";
        emit logger(GateCaption + "\t | passdrop:\t\t\t\t | ip: " + addr + " | barcode: " + barcode);
    }


    QThread db_worker_thread;
    QString ticket_state_description(int desc)
    {
        switch(desc)
        {
        case 0x1000: return "Слава Хозяину!";
        case 0x0200: return "Турникет не зарегистрирован!";
        case 0x0300: return "Билет не зарегистрирован на мероприятии!";
        case 0x0100: return "Выход разрешен.";
        case 0x0110: return "Выход запрещен! Билет еще не прошел!";
        case 0x0120: return "Выход запрещен! Билет уже покинул мероприятие!";
        case 0x0130: return "Выход запрещен! Неизвестное состояние билета!";
        case 0x0000: return "Вход разрешен.";
        case 0x0001: return "Вход запрещен! Билет уже на мероприятии!";
        case 0x0002: return "Вход запрещен! Неизвестное состояние билета";
        default :
            return "Неизвестный статус!";
        }
    }


signals:
    void logger( QString );
    void send_to_socket(message);
    void send_to_linked_socket(message);
    void disconnect(QString);

    void get_register_from_wicket(logical_reader * );
    void get_barcode_from_wicket(logical_reader * );
    void remove_bc_from_check(QString);

    void request_check_ticket_SIG(QString);
    void request_confirm_pass_SIG(int);
    void count(int);

    void send_to_widgets( QJsonObject );

};

#include "db_worker.h"

#endif // WICKET_OBJECT_H
