// Copyright (C) 2010-2013 Michael Wiklund.
// All rights reserved.
// Contact: arnlib@wiklunden.se
//
// This file is part of the WebArnBrowser - Active Registry Network Browser.
// Parts of WebArnBrowser depend on Qt 4 and/or other libraries that have their own
// licenses. WebArnBrowser is independent of these licenses; however, use of these other
// libraries is subject to their respective license agreements.
//
// GNU Lesser General Public License Usage
// The dependent library Wt is not using LGPL.
// This file may be used under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation and
// appearing in the file LICENSE.LGPL included in the packaging of this file.
// In addition, as a special exception, you may use the rights described
// in the Nokia Qt LGPL Exception version 1.1, included in the file
// LGPL_EXCEPTION.txt in this package.
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public License version 3.0 as published by the Free Software Foundation
// and appearing in the file LICENSE.GPL included in the packaging of this file.
//
// Other Usage
// Alternatively, this file may be used in accordance with the terms and
// conditions contained in a signed written agreement between you and Michael Wiklund.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//

/*******
 **
 **  Run With: --docroot . --http-address 0.0.0.0 --http-port 2012 arnHost=myServer arnBrowseName=Test
 **
 *******/

#include <iostream>
#include <signal.h>

#include <Wt/WApplication>
#include <Wt/WServer>
#include <Wt/WFileResource>

#include "MainWindow.hpp"
#include "QtMainThread.hpp"
#include <ArnLib/XStringMap.hpp>
#include <QMutex>
#include <QCoreApplication>
#include <QDebug>


using namespace Wt;

QByteArray  arnBrowseName;
QMutex  mainMutex;


WApplication*  createApplication( const WEnvironment& env)
{
    qDebug() << "--- createApp: Thr=" << QThread::currentThread();

    return new MainWindowW(env);
}


int  main( int argc, char *argv[])
{
    // Needed for Qt's eventloop threads to work
    QCoreApplication  app( argc, argv);

    QByteArray  argXs;
    for (int i = 0; i < argc; ++i) {
        if (i > 0)  argXs += " ";
        argXs += argv[i];
    }
    XStringMap  argXsm( argXs);
    arnBrowseName   = argXsm.value("arnBrowseName");
    QString arnHost = argXsm.value("arnHost");

    QtMainThread::instance().setArnConnection( arnHost);
    QtMainThread::instance().start();

    try {
        // use argv[0] as the application name to match a suitable entry
        // in the Wt configuration file, and use the default configuration
        // file (which defaults to /etc/wt/wt_config.xml unless the environment
        // variable WT_CONFIG_XML is set)
        WServer  server( argv[0]);

        // WTHTTP_CONFIGURATION is e.g. "/etc/wt/wthttpd"
        server.setServerConfiguration( argc, argv, WTHTTP_CONFIGURATION);

        // add a single entry point, at the default location (as determined
        // by the server configuration's deploy-path)
        server.addEntryPoint( Wt::Application, createApplication);

        WFileResource  logoutPage("text/html", "logout.html");
        server.addResource( &logoutPage, "/logout");

        Session::configureAuth();

        if (server.start()) {
            int  sig = WServer::waitForShutdown( argv[0]);

            std::cerr << "Shutdown (signal = " << sig << ")" << std::endl;
            server.stop();

            if (sig == SIGHUP)
              WServer::restart( argc, argv, environ);
        }
    }
    catch (WServer::Exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
        return 1;
    }

    // MW: Should shutdown qtMainThread ...
}

