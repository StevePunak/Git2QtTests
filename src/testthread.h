#ifndef TESTTHREAD_H
#define TESTTHREAD_H
#include "credentialresolver.h"

#include <Kanoop/utility/abstractthreadclass.h>
#include <git2qt.h>

class TestThread : public AbstractThreadClass
{
public:
    TestThread(const QString& localPath);

    // AbstractThreadClass interface
private:
    virtual void threadStarted() override;

    void openRepository();

    void aheadBehindTest();
    void checkoutTest();
    void mergeTest();
    void tagTest();
    void headCommitTest();
    void deleteReferenceTest();
    void amendMessageTest();
    void submoduleTest();
    void diffTest();
    void pullTest();

    QString _localPath;
    CredentialResolver _credentialsResolver;

    QString _remoteUrl;

    GIT::Repository* _repository = nullptr;

private slots:
    void onProgress(uint32_t receivedBytes, uint32_t receivedObjects, uint32_t totalObjects);
};

#endif // TESTTHREAD_H
