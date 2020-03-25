#pragma once

#include "EventsStorage/IEventsStorage.h"

#include <QtSql/QSqlDatabase>

#include <map>
#include <mutex>
#include <experimental/filesystem>
#include <string>

namespace Challenge::EventsStorage {

    class SqliteStorage : public IEventsStorage {
        public:
             //! constructs database working on file
             SqliteStorage( std::experimental::filesystem::path _absPathToDbFile ); // may throw std::runtime_error

             //! constructs database on memory
             SqliteStorage(); // may throw std::runtime_error
             ~SqliteStorage() override = default;

            bool saveEvent( const EventData& _event ) override;
            std::optional<Events> getSavedEvents(uint64_t _firstEvent, uint64_t _lastEvent) const override;
            std::optional<uint64_t> getNumberOfEvents() const override;
            bool registerEventAddedCallback( EventSavedCallback _callback, void* _key ) override;

        private:
            void initializeDatabase( const std::string& _sqliteName ); // may throw std::runtime_error


        private:
            QSqlDatabase m_database;
            using CallbackRegister = std::unordered_map<void*, EventSavedCallback >;
            CallbackRegister m_callbacks;

            std::mutex m_callbackMutex;
    };

} // namespace Challenge::EventsStorage
