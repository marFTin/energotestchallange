#pragma once

#include <QMainWindow>

#include "uic/ui_MainWindow.h"

#include "Lib/TableEventsModel/TableEventsModel.h"

#include <QPointer>

#include <memory>

namespace Challenge {

    namespace  Communication {
    namespace Client {
        class ITransportConnectivityManager;
        class ITransportConnection;
        class IHandshake;
        class IProtocolExecutor;
    } // namespace Client
    } // namespace Communication

namespace Application {

        //! Mainwindow of the application
        class MainWindow : public QMainWindow {
        Q_OBJECT
        public:
            using Ui = Ui::MainWindow;

            //! Constructor
            /*!
             *
             * @param _parent parent window
             * @throw std::runtime_error in case of error
             */
            explicit MainWindow(QWidget *_parent = nullptr);

        public slots:
            void onButtonConnectClicked();
            void onButtonSendNewEventClicked();
            void updateWindow();

        private:
            void onConnectionExpired();

        private:
            Ui m_ui;

            std::shared_ptr< Communication::Client::ITransportConnectivityManager > m_connectivityManager;
            std::shared_ptr< Communication::Client::IHandshake > m_handshake;
            std::shared_ptr< Communication::Client::IProtocolExecutor > m_protocolExecutor;
            QPointer<TableEventsModel> m_tableModel;
        };

} // Challenge
} // Application
