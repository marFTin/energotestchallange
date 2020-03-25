#include "SqliteStorage.h"

#include "Lib/Log/Logger.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QString>
#include <QVariant>

#include <cassert>
#include <stdexcept>

namespace Challenge::EventsStorage {

    //table events(id,text,timestamp(time from epoch),priority)
    constexpr auto SQL_CREATE_EVENTS_TABLE =
            "CREATE TABLE IF NOT EXISTS events (id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, text TEXT NOT NULL, timestamp INTEGER NOT NULL, priority INTEGER NOT NULL)";

    constexpr auto SQL_INSERT_EVENT = "INSERT INTO events(text,timestamp,priority) "
                                      "VALUES(?,?,?)";

    constexpr auto SQL_GET_NUMBER_OF_EVENTS = "SELECT COUNT(id) FROM events";

    constexpr auto SQL_GET_EVENTS = "SELECT text,timestamp,priority FROM events WHERE id >= ? AND id <= ?";


    template<>
    std::shared_ptr<IEventsStorage> IEventsStorage::create<>() try {
        constexpr auto DB_FILE_ABS_PATH = "/tmp/challenge.db";
        return std::unique_ptr<IEventsStorage>( new SqliteStorage(DB_FILE_ABS_PATH) );

    } catch ( std::exception& _exception ) {
        LOG_ERROR( _exception.what() );
        return nullptr;
    }

    template std::shared_ptr<Challenge::EventsStorage::IEventsStorage> Challenge::EventsStorage::IEventsStorage::create();

SqliteStorage::SqliteStorage( std::experimental::filesystem::path _absPathToDbFile ) {
    if ( !_absPathToDbFile.is_absolute() ) {
        throw std::runtime_error( "Path to file is not absolute" );
    }
    initializeDatabase(_absPathToDbFile.c_str());

    assert( m_database.isValid() );
    assert( m_database.isOpen() );
}

SqliteStorage::SqliteStorage() {
    initializeDatabase(":memory:");
    assert( m_database.isValid() );
    assert(m_database.isOpen());
}

void
SqliteStorage::initializeDatabase( const std::string& _sqliteName ) {
    assert( !_sqliteName.empty() );

    const QString driver("QSQLITE");

    if ( !QSqlDatabase::isDriverAvailable( driver ) ) {
        throw std::runtime_error("Sqlite driver is not available");
    }

    m_database  = QSqlDatabase::addDatabase(driver);
    m_database.setDatabaseName(_sqliteName.c_str());

    if ( !m_database.open() ) {
        throw std::runtime_error("Cannot open sqlite db database");
    }

    QSqlQuery queryCreateEventsTable(m_database);
    queryCreateEventsTable.prepare(SQL_CREATE_EVENTS_TABLE);
    if ( !queryCreateEventsTable.exec() ) {
        throw std::runtime_error( queryCreateEventsTable.lastError().text().toStdString() + " Cannot create events table");
    }

}

bool
SqliteStorage::saveEvent( const EventData& _event ) {
    assert( m_database.open() );

    QSqlQuery query(m_database);
    query.prepare( SQL_INSERT_EVENT );

    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(_event.timeStamp.time_since_epoch() ).count();

    query.addBindValue( QString::fromStdString( _event.text ) );
    query.addBindValue( QVariant::fromValue(timestamp) );
    query.addBindValue( QVariant::fromValue(_event.priority) );

    if(!query.exec()) {
        LOG_ERROR( query.lastError().text().toStdString().c_str() );
        return false;
    }

    std::lock_guard lock(m_callbackMutex);
    for ( auto& callback : m_callbacks ) {
        assert(callback.second);
        callback.second();
    }
    return true;
}

std::optional<IEventsStorage::Events>
SqliteStorage::getSavedEvents(uint64_t _firstEvent, uint64_t _lastEvent) const {
    if ( _firstEvent > _lastEvent ) {
        return std::nullopt;
    }

    // SQL count from 1, we count events from 0
    auto firstEvent = _firstEvent + 1;
    auto lastEvent = _lastEvent == LAST_EVENT_NUMBER ? LAST_EVENT_NUMBER : _lastEvent + 1;

    QSqlQuery query(m_database);
    query.prepare(SQL_GET_EVENTS);
    query.addBindValue( QVariant::fromValue( firstEvent ));
    query.addBindValue( QVariant::fromValue( lastEvent ));

    if ( !query.exec() ) {
        LOG_ERROR( query.lastError().text().toStdString().c_str() );
        return std::nullopt;
    }

    IEventsStorage::Events events;
    while ( query.next() ) {
        QString text = query.value(0).toString();
        uint64_t  timestamp = query.value(1).toULongLong();
        uint32_t priority = query.value(2).toUInt();

        std::chrono::time_point<std::chrono::system_clock> time( std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(timestamp) ) );
        // I have no time to fight now with emplace_back and chrono, at now it is passed by copy
        EventData data{time, text.toStdString(), priority};
        events.push_back( std::move(data) );
    }

    return std::move(events);
}

std::optional<uint64_t>
SqliteStorage::getNumberOfEvents() const {
    QSqlQuery query( SQL_GET_NUMBER_OF_EVENTS, m_database);

    if ( !query.isActive() ) {
        LOG_ERROR( query.lastError().text().toStdString().c_str() );
        return std::nullopt;
    }

    if ( !query.next() ) {
        return std::nullopt;
    }

    return query.value(0).toULongLong();
}

bool
SqliteStorage::registerEventAddedCallback( EventSavedCallback _callback, void* _key ) {
    std::lock_guard lock(m_callbackMutex);
    if ( _callback == nullptr ) {
        m_callbacks.erase(_key);
        return true;
    }
    return m_callbacks.insert_or_assign( _key, _callback ).second;
}

} // namespace Challenge::EventsStorage
