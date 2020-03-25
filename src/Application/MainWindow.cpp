#include "MainWindow.h"

#include "Communication/Client/TransportConnectivityManager/ITransportConnectivityManager.h"
#include "Communication/Client/TransportConnectivityManager/ITransportConnection.h"
#include "Communication/Client/IHandshake.h"
#include "Communication/Client/IProtocolExecutor.h"

#include "Lib/C++Tools/ScopedAction.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>

#include <cassert>
#include <stdexcept>


namespace Challenge::Application {

MainWindow::MainWindow(QWidget* _parent ) : QMainWindow(_parent) {
    m_ui.setupUi(this);

    m_connectivityManager = Communication::Client::ITransportConnectivityManager::create();

    if ( !m_connectivityManager ) {
        throw std::runtime_error( "Cannot create connectivity manager" );
    }

    updateWindow();
}

void
MainWindow::onButtonConnectClicked() {
    assert( !m_handshake );

    ScopedAction action( [this]{ updateWindow(); });

    auto transportConnection = m_connectivityManager->connectToServer();

    if ( !transportConnection ) {
        QMessageBox::information( this, "Cannot connect to server", "Problems with connection to server" );
        return;
    }

    transportConnection->registerConnectionExpiredCallback( [this]{onConnectionExpired();} );
    m_handshake = Communication::Client::IHandshake::start( transportConnection );

    if (!m_handshake) {
        QMessageBox::information( this, "Cannot connect to server", "Problems with handshake with server" );
        return;
    }

    m_protocolExecutor = Communication::Client::IProtocolExecutor::create(m_handshake);
    if ( !m_protocolExecutor ) {
        QMessageBox::information( this, "Cannot connect to server", "Problems with protocol initialization" );
        m_handshake.reset();
        return;
    }

    m_ui.m_eventsView->setModel(nullptr );
    m_tableModel = new TableEventsModel( m_protocolExecutor, this );
    m_ui.m_eventsView->setModel(m_tableModel);

    m_ui.m_eventsView->horizontalHeader()->setSectionResizeMode(
            static_cast<int32_t>(TableEventsModel::Columns::Text), QHeaderView::Stretch);
}

void
MainWindow::onButtonSendNewEventClicked() {
   if ( !m_handshake ) {
       return;
   }

   if ( !m_protocolExecutor ) {
       return;
   }

   auto result = m_protocolExecutor->sendEvent( m_ui.m_eventTextEdit->toPlainText().toLocal8Bit().data(), m_ui.m_spinPriority->value() );

   if ( !result ) {
       QMessageBox::information( this, "Cannot send event", "Problem with sending event occurred" );
       return;
   }

   m_ui.m_eventTextEdit->clear();
   updateWindow();
}


void
MainWindow::updateWindow() {
    m_ui.m_connectionStatus->setText( m_handshake ? "Connected" : "Not Connected" );
    m_ui.m_connectButton->setEnabled( !m_handshake );
    m_ui.m_eventsBrowserWidget->setEnabled( m_handshake != nullptr );
    m_ui.m_eventsCreatorWidget->setEnabled( m_handshake != nullptr );
    m_ui.m_sendEventButton->setEnabled( m_handshake != nullptr && !m_ui.m_eventTextEdit->toPlainText().isEmpty() );
}

void
MainWindow::onConnectionExpired() {
    m_handshake.reset();
    updateWindow();
}

} //namespace Challenge::Application