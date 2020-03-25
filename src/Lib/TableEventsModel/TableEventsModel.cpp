#include "Lib/TableEventsModel/TableEventsModel.h"

#include "Communication/Client/IProtocolExecutor.h"

#include <cassert>
#include <sstream>

namespace Challenge {


TableEventsModel::TableEventsModel(std::shared_ptr<Communication::Client::IProtocolExecutor> _protocolExecutor, QObject *_parent)
    : m_protocolExecutor( _protocolExecutor )
    , QAbstractTableModel(_parent) {
    if ( !m_protocolExecutor ) {
        throw std::runtime_error( "Null protocol executor" );
    }

    auto onNewEventAddedCallback = [this](uint64_t _numberOfEvents){ onNewEventAdded(_numberOfEvents); };
    m_protocolExecutor->registerNewEventAddedCallback( onNewEventAddedCallback );

}

int32_t TableEventsModel::rowCount(const QModelIndex &_parent) const {
    assert( m_protocolExecutor );

    if ( m_cachedRowCount.has_value() ) {
        return m_cachedRowCount.value();
    }

    auto numberOfSavedEvents = m_protocolExecutor->getNumberOfSavedEvents();

    if ( !numberOfSavedEvents.has_value() ) {
        return 0;
    }

    m_cachedRowCount = numberOfSavedEvents.value();
    return numberOfSavedEvents.value();
}

int32_t TableEventsModel::columnCount(const QModelIndex &_parent) const {
    return 3;
}

QVariant TableEventsModel::data(const QModelIndex &_index, int _role) const {
    assert(m_protocolExecutor);

    if ( _role != Qt::DisplayRole ) {
        return QVariant();
    }

    auto cacheIt = m_cache.find( _index.row() );
    EventData eventData{};

    if ( cacheIt != m_cache.end() ) {
        eventData = cacheIt->second;
    } else {
        auto events = m_protocolExecutor->getSavedEvents(_index.row(), _index.row());

        if (!events.has_value()) {
            return QVariant();
        }

        if (events.value().empty()) {
            return QVariant();
        }
        eventData = events.value()[0];
        m_cache.insert( std::make_pair(_index.row(), eventData) );
    }

    switch ( _index.column() ) {
        case static_cast<int32_t>(Columns::Priority) :
            return eventData.priority;
        case static_cast<int32_t>(Columns::Text) :
            return QString::fromStdString( eventData.text );
        case static_cast<int32_t>(Columns::TimeStamp): {
            std::time_t timeC = std::chrono::system_clock::to_time_t(eventData.timeStamp);
            std::stringstream stream;
            stream << std::ctime(&timeC);
            return QString::fromStdString( stream.str() );
        }
        default:
            assert( !"unsupported column" );
            return "Unsupported Column";
    }
}

QVariant
TableEventsModel::headerData(int _section, Qt::Orientation _orientation, int _role) const {

    if (_role == Qt::DisplayRole && _orientation == Qt::Horizontal) {
        switch (_section) {
            case static_cast<int32_t>(Columns::Priority) :
                return QString( "Priority" );
            case static_cast<int32_t>(Columns::Text) :
                return QString( "Text" );
            case static_cast<int32_t>(Columns::TimeStamp): {
                return QString( "Timestamp" );
            }
            default:
                assert( !"unsupported column" );
                return "Unsupported Column";
        }
    }
    return QVariant();
}

void
TableEventsModel::onNewEventAdded(uint64_t _numberOfEvents) {
    m_cachedRowCount.reset();
    beginResetModel();
    endResetModel();
}
} // namespace Challenge
