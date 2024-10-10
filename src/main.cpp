#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

#include <git2qt/repository.h>

#include "examples.h"
#include "testexception.h"
#include "testthread.h"

const QString keyHelp               = "help";
const QString keyLocalPath          = "local-path";
const QString keyVerbose            = "verbose";

const QString verbAuto              = "auto";
const QString verbClone             = "clone";
const QString verbExamples          = "examples";
const QString verbTest              = "test";

const QStringList VALID_VERBS =
{
    verbAuto,
    verbClone,
    verbExamples,
    verbTest,
};

QString toDelimitedString(const QStringList &list, char delimiter)
{
    QString outputString;
    outputString.reserve(list.size() * 256);
    QTextStream output(&outputString);
    for(int i = 0;i < list.count();i++)
    {
        output << list.at(i);
        if(i < list.count() - 1)
            output << delimiter;
    }
    return outputString;
}

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    application.setApplicationName("Git2QtTests");
    application.setApplicationVersion(QT_STRINGIFY(VERSION));
    application.setOrganizationDomain("punak.com");
    application.setOrganizationName("Stephen Punak, Inc.");

    QTextStream standardError(stderr);

    // parse command line
    QCommandLineParser parser;
    parser.setApplicationDescription(QString("Git2QtTests v%1").arg(application.applicationVersion()));

    parser.addOptions({
        // -------------------- Short Options
        // Short,Long                   // Description
        {{ "v",     keyVerbose, },      "Verbose output",                                   /** short option */                            },
        {{ "?",     keyHelp, },         "Print usage and exit",                             /** short option */                            },

        // -------------------- Long Options
        // Short,Long                   // Description                                      // Key name                 // Default
        {{ keyLocalPath, },             "Local repo path",                                   keyLocalPath,                                 },
    });

    parser.process(application);

    try
    {
        // Quick exit options
        if(parser.isSet(keyHelp)) {
            parser.showHelp(0);
        }

        if(parser.positionalArguments().count() == 0) {
            throw TestException(QString("No action given. Must be one of:  (%1)").arg(toDelimitedString(VALID_VERBS, ',')));
        }

        QString localPath = parser.value(keyLocalPath);
        if(localPath.isEmpty()) {
            throw TestException("--local-path must be set");
        }

        QString verb = parser.positionalArguments().at(0);
        if(VALID_VERBS.contains(verb) == false) {
            throw TestException(QString("Invalid verb:  (%1)").arg(verb));
        }

        if(verb == verbExamples) {
            Examples examples(localPath);
            examples.unstageAllFiles();
        }
        else if(verb == verbAuto) {
            Examples examples(localPath);
            examples.autoTest();
        }
        else if(verb == verbTest) {
            TestThread t(localPath);
            t.start();
            t.waitForCompletion();
        }
        else if(verb == verbClone) {
            QFileInfo fileInfo(localPath);
            if(fileInfo.absoluteDir().exists() == false) {
                throw TestException(QString("Parent of local-path %1 does not exist").arg(localPath));
            }
            Examples::cloneRepo("https://github.com/StevePunak/GitTesting.git", localPath);
        }
    }
    catch(const TestException& e)
    {
        standardError << QString("EXCEPTION: %1").arg(e.message()) << Qt::endl;
        parser.showHelp(-1);
    }

    return 0;
}
