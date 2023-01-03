#include <logical_reader.h>

void logical_reader::safe_delete_db_worker()
{
    QSqlDatabase::removeDatabase(this->_db_worker->reader_ptr_to_string_for_db_connection_id);
    db_worker_thread.exit(0);
}

void logical_reader::create_DB_worker()
{
    _db_worker = new db_worker(this, GateCaption, conn_str);
    connect(_db_worker, &db_worker::check_ticket_result, this, &logical_reader::slot_check_ticket_result);
    connect(this,  &logical_reader::request_check_ticket_SIG, _db_worker, &db_worker::check_ticket);
    connect(this,  &logical_reader::request_confirm_pass_SIG, _db_worker, &db_worker::confirm_pass);

    connect(_db_worker, &db_worker::count, this,  &logical_reader::count);
    connect(_db_worker, &db_worker::logger, this,  &logical_reader::logger);
    connect(&db_worker_thread, &QThread::started, _db_worker, &db_worker::create_db_connect);
    connect(&db_worker_thread, &QThread::finished, this, &logical_reader::deleteLater);
    _db_worker->moveToThread(&db_worker_thread);
    db_worker_thread.start(QThread::HighestPriority);
//    return _db_worker;
}
