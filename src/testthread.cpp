#include "testthread.h"

#include <Kanoop/commonexception.h>

using namespace GIT;

TestThread::TestThread(const QString& localPath) :
    AbstractThreadClass("testthread"),
    _localPath(localPath),
    _credentialsResolver("spunak", "xxx", "/home/spunak/.ssh/id_ed25519.pub", "/home/spunak/.ssh/id_ed25519")
{
    _remoteUrl = "git@github.com:StevePunak/GitTesting.git";

}

void TestThread::threadStarted()
{
    try
    {
        openRepository();

        pullTest();

        finishAndStop(true, "Success");
    }
    catch(const CommonException& e)
    {
        finishAndStop(false, e.message());
    }
}

void TestThread::openRepository()
{
    bool isRepo = Repository::isRepository(_localPath); Q_UNUSED(isRepo);
    _repository = new Repository(_localPath);
    if(_repository->isNull()) {
        throw CommonException(_repository->errorText());
    }

    connect(_repository, &Repository::progress, this, &TestThread::onProgress);
    _repository->setCredentialResolver(&_credentialsResolver);

}

void TestThread::pullTest()
{
    Signature signature = _repository->config()->buildSignature();
    _repository->pull(signature);
}

void TestThread::onProgress(uint32_t receivedBytes, uint32_t receivedObjects, uint32_t totalObjects)
{
    logText(LVL_DEBUG, LVL1(), QString("onProgress: received %1 bytes, and objects %2 of %3").arg(receivedBytes).arg(receivedObjects).arg(totalObjects));
}


