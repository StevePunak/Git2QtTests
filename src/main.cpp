/******************************************************************************************
**
** main.cpp
**
** Copyright (C) Tesseract Engineering, Inc - All Rights Reserved
**
** This source code is protected under international copyright law.  All rights
** reserved and protected by the copyright holders.
**
** This file is confidential and only available to authorized individuals with the
** permission of the copyright holders.  If you encounter this file and do not have
** permission, please contact the copyright holders and delete this file.
**
** Author:  Stephen Punak
** Created: Fri Oct 6 12:58:54 2023
**
******************************************************************************************/
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>
#include "generaltestthread.h"
#include "ipctestthread.h"
#include "testudpclient.h"
#include "testudpserver.h"
#include <tess/log.h>
#include <tess/diag/uds.h>
#include <tess/softwareversion.h>
#include <tess/tessexception.h>
#include <Kanoop/stringutil.h>

const QString keyHelp               = "help";
const QString keyVerbose            = "verbose";

const QString verbTest              = "test";
const QString verbUdpClient         = "udp-client";
const QString verbUdpServer         = "udp-server";

const QStringList VALID_VERBS =
{
    verbTest,
    verbUdpClient,
    verbUdpServer,
};


int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    application.setApplicationName("testutil");
    application.setApplicationVersion(SoftwareVersion(QT_STRINGIFY(VERSION)).toString());
    application.setOrganizationDomain("tesseng.com");
    application.setOrganizationName("Tesseract Engineering, Inc.");

    QTextStream standardError(stderr);

    // parse command line
    QCommandLineParser parser;
    parser.setApplicationDescription(QString("Test Utility v%1").arg(application.applicationVersion()));

    parser.addOptions({
                       // -------------------- Short Options
                       // Short,Long                   // Description
                       {{ "v",     keyVerbose, },      "Verbose output",                                   /** short option */                            },
                       {{ "?",     keyHelp, },         "Print usage and exit",                             /** short option */                            },

                       // -------------------- Long Options
                       // Short,Long                   // Description                                      // Key name                 // Default
//                       {{ keyLogLevel, },              "Log level",                                        keyLogLevel,                "info",            },
                       });

    parser.process(application);

    try
    {
        // Quick exit options
        if(parser.isSet(keyHelp)) {
            parser.showHelp(0);
        }

        // Set up logging
        Log::setLevel(Log::Debug);
        Log::LogCategory::setDefaultLogLevel(Log::Debug);
        Log::enableOutputFlags(Log::Console);


        if(parser.positionalArguments().count() == 0) {
            throw TessException(QString("No action given. Must be one of:  (%1)").arg(StringUtil::toDelimitedString(VALID_VERBS, ',')));
        }

        QString verb = parser.positionalArguments().at(0);
        if(VALID_VERBS.contains(verb) == false) {
            throw TessException(QString("Action must be one of:  (%1)").arg(StringUtil::toDelimitedString(VALID_VERBS, ',')));
        }

        if(verb == verbTest) {
            GeneralTestThread thread;
            thread.start();
            thread.waitForCompletion(TimeSpan::fromDays(1));
        }
        else if(verb == verbUdpServer) {
            TestUdpServer server(QHostAddress::Any, 9001);
            server.start();
            server.waitForCompletion(TimeSpan::fromDays(1));
        }
        else if(verb == verbUdpClient) {
            TestUdpClient client;
            client.start();
            client.waitForCompletion(TimeSpan::fromDays(1));
        }
    }
    catch(const TessException& e)
    {
        standardError << QString("EXCEPTION: %1").arg(e.message()) << Qt::endl;
        parser.showHelp(-1);
    }

    return 0;
}
