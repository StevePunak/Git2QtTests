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

        checkoutTest();
        // mergeTest();
        // tagTest();
        // headCommitTest();
        // deleteReferenceTest();
        // amendMessageTest();
        // submoduleTest();
        // diffTest();
        // pullTest();

        finishAndStop(true, "Success");
    }
    catch(const CommonException& e)
    {
        finishAndStop(false, e.message());
    }
}

void TestThread::openRepository()
{
_localPath = "/home/spunak/tmp/gitlab/DestRepo";
    bool isRepo = Repository::isRepository(_localPath); Q_UNUSED(isRepo);
    _repository = new Repository(_localPath);
    if(_repository->isNull()) {
        throw CommonException(_repository->errorText());
    }

    connect(_repository, &Repository::progress, this, &TestThread::onProgress);
    _repository->setCredentialResolver(&_credentialsResolver);

}

void TestThread::checkoutTest()
{
    _repository->checkoutLocalBranch("develop");
}

void TestThread::mergeTest()
{
    Signature author = _repository->config()->buildSignature();
    QString sourceBranchName = "feature/test-branch";
    QString targetBranchName = _repository->currentBranch().friendlyName(true);
    QString message = QString("Merge branch '%1' into %2").arg(sourceBranchName).arg(targetBranchName);
    _repository->merge("feature/test-branch", author, message);
}

void TestThread::tagTest()
{
    Tag::ConstPtrList tags = _repository->tags_DEP();
}

void TestThread::headCommitTest()
{
    Commit head = _repository->headCommit();
    Commit::List commits = _repository->allCommits(SortStrategyTime);
    Q_UNUSED(head)
    Q_UNUSED(commits)
}

void TestThread::deleteReferenceTest()
{
    ReferenceList refs = _repository->findReferences(QRegularExpression("^refs/original"));
    for(const Reference& ref : refs) {
        _repository->deleteLocalReference(ref);
    }
}

void TestThread::amendMessageTest()
{
    Commit commit = _repository->findCommit("86ca3d8aa942301dd6fbd4d90bb1e7aeb73ac385");
    _repository->amendCommitMessage(commit, "This is the amended message");
}

void TestThread::submoduleTest()
{
    ConfigurationEntry::List configEntries = _repository->config()->getAll();
    for(const ConfigurationEntry& entry : configEntries) {
        logText(LVL_DEBUG, QString("%1 [%2]").arg(entry.key()).arg(entry.value().toString()));
    }

    Submodule::List subs = _repository->submodules();
    Submodule module = subs.findByName("LidarTest");
    _repository->deleteSubmodule(module);
    logText(LVL_DEBUG, "Here");
}

void TestThread::diffTest()
{
    Commit a = _repository->findCommit("62c2aad1af49fccd275cd2cba08e895f9c0d6990");
    Commit b = _repository->findCommit("5c5754075d170209836ceb5d08e28f0512b4f595");

    TreeEntry treeEntryA = TreeEntry::findForCommit(a, "README.new");
    TreeEntry treeEntryB = TreeEntry::findForCommit(b, "README.new");
    Q_UNUSED(treeEntryA) Q_UNUSED(treeEntryB)

    TreeChanges changes = _repository->diff()->compare(a, b, CompareOptions(), DiffModifier::DiffModNone);

    // Tree tipTree = _repository->head().tip().tree();
    // Tree thisTree = a.tree();
    // Tree parentTree = b.tree();
    // TreeChanges changes = _repository->diff()->compare(parentTree, thisTree);
    Q_UNUSED(changes)

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


