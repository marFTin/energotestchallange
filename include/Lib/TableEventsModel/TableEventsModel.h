#pragma once

#include <QAbstractTableModel>

#include "Event/EventData.h"

#include <cinttypes>
#include <memory>
#include <unordered_map>

namespace Challenge {

    namespace Communication {
    namespace Client {
            class IProtocolExecutor;
    } // namespace Client
    } // namespace Communication

    class TableEventsModel : public QAbstractTableModel {
    Q_OBJECT
    public:
        enum class Columns{
            TimeStamp, Priority, Text
        };

        //! Constructor, will throw when _protocolExecutor it nullptr
        TableEventsModel( std::shared_ptr<Communication::Client::IProtocolExecutor> _protocolExecutor,  QObject* _parent = nullptr);
        int32_t rowCount(const QModelIndex& _parent = QModelIndex()) const override;
        int32_t columnCount(const QModelIndex& _parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& _index, int _role ) const override;
        QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;

    private:
        void onNewEventAdded( uint64_t _numberOfEvents);

    private:
        using Cache = std::unordered_map< uint64_t, EventData >;
        mutable std::optional< int32_t > m_cachedRowCount;
        mutable Cache m_cache;
        std::shared_ptr<Communication::Client::IProtocolExecutor> m_protocolExecutor;

    };

} // Challenge
