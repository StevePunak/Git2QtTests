#include "examples.h"
#include "testexception.h"
#include <QFile>
#include <git2qt.h>

using namespace GIT;

Examples::Examples(const QString& localPath) :
    _remoteUrl("https://github.com/StevePunak/GitTesting.git"),
    _localPath(localPath),
    _stdout(stdout),
    _repository(nullptr)
{
    initializeRepo();
}

void Examples::autoTest()
{
    try
    {
        // Dump all the branches
        listBranches();

        // Dump all the indexes
        listIndexes();

        // Need to start with nothing modified
        verifyNothingModified();

        // Modify, stage, unstage and restore
        stageAndUnstageSomeFiles();

        // Commit some changes the to hard reset back to original HEAD
        commitSomeChangesAndResetHard();

        // Create a lightweight tag, search for it, then delete it
        createAndDeleteLightweightTag();

        // Create an annotated tag, search for it, then delete it
        createAndDeleteAnnotatedTag();

        // Modify some files and dump the diffs, the restore the repo
        createAndDumpSomeDiffs();
    }
    catch(TestException& e)
    {
        throw TestException(QString("%1 [%2]").arg(e.message()).arg(_repository->errorText()));
    }
}

void Examples::cloneRepo(const QString& remoteUrl, const QString& localPath)
{
    ExampleCredentialResolver credsResolver;
    ExampleProgressCallback progressCallback;

    Repository* repo = Commands::clone(remoteUrl, localPath, &credsResolver, &progressCallback);
    if(repo != nullptr) {
        qDebug() << "Successfully cloned " << remoteUrl << " into " << localPath;
        delete repo;
    }
    else {
        throw TestException(Commands::lastErrorText());
    }
}

void Examples::listBranches()
{
    for(const Branch& branch : _repository->branches()) {
        _stdout << QString("Branch: %1  Canonical Name: %2  Friendly Name: %3  Type: %4  Ref Type: %5")
                   .arg(branch.name()).arg(branch.canonicalName()).arg(branch.friendlyName())
                   .arg(getBranchTypeString(branch.branchType()))
                   .arg(getReferenceTypeString(branch.reference().type()));
        _stdout << Qt::endl;
    }

    if(_repository->branches().findLocalBranch("develop").isValid() == false) {
        throw TestException("Failed to find local branch develop");
    }

}

void Examples::listIndexes()
{
    for(const IndexEntry& entry : _repository->index()->entries()) {
        _stdout << QString("Object ID: %1  Mode: %2  Stage Level: %3  Path: %4")
                   .arg(entry.objectId().toString())
                   .arg(getModeString(entry.mode()))
                   .arg(getStageLevelString(entry.stageLevel()))
                   .arg(entry.path());
        _stdout << Qt::endl;
    }

    if(_repository->index()->entries().findByObjectId("304c4c126c442ceef235c78fa6a0af55ccbe7204").isValid() == false) {
        throw TestException("Failed to find object");
    }
}

void Examples::verifyNothingModified()
{
    StatusEntry::List modified = _repository->status().modified();
    if(modified.count() > 0) {
        throw TestException(QString("Expecting no modified entries but found %1").arg(modified.count()));
    }
}

/**
 * @brief Examples::stageAndUnstageSomeFiles
 *
 * 1. Modify a file
 * 2. Stage the file
 * 3. Unstage the file
 */
void Examples::stageAndUnstageSomeFiles()
{
    verifyNothingModified();

    QString filename = "subdir/testclass1.cpp";
    modifyFile(filename);

    StatusEntry::List modified = _repository->status().modified();
    if(modified.count() != 1 || modified.at(0).path() != filename) {
        throw TestException("Did not find the expected modifications");
    }

    if(_repository->stage(modified) == false) {
        throw TestException("Failed to stage file");
    }

    if(_repository->status().staged().count() != 1) {
        throw TestException("Staged file count != 1");
    }

    if(_repository->unstage(modified.paths()) == false) {
        throw TestException("Failed to unstage file");
    }

    if(_repository->restore(QStringList() << filename) == false) {
        throw TestException("Failed to restore repository");
    }
}

void Examples::unstageAllFiles()
{
    StatusEntry::List staged = _repository->status().staged();
    if(staged.count() > 0) {
        if(_repository->unstage("*") == false) {
            throw TestException("Failed to unstage file");
        }
    }
}

void Examples::commitSomeChangesAndResetHard()
{
    verifyNothingModified();

    ObjectId headObjectId = _repository->head().reference().objectId();
    Commit headCommit = _repository->findCommit(headObjectId);
    if(headCommit.isNull()) {
        throw TestException("Failed to find head commit");
    }

    QString filename = "subdir/testclass1.cpp";
    modifyFile(filename);

    StatusEntry::List modified = _repository->status().modified();
    if(modified.count() != 1 || modified.at(0).path() != filename) {
        throw TestException("Did not find the expected modifications");
    }

    if(_repository->stage(modified) == false) {
        throw TestException("Failed to stage file");
    }

    if(_repository->status().staged().count() != 1) {
        throw TestException("Staged file count != 1");
    }

    Signature signature("beavis", "beavis@butthead.com");
    Commit commit = _repository->commit("Temporary commit for testing", signature, signature);
    if(commit.isNull()) {
        throw TestException("Failed to commit");
    }

    if(_repository->reset(headCommit, ResetHard) == false) {
        throw TestException("Failed to reset");
    }
}

void Examples::createAndDeleteLightweightTag()
{
    Commit commit = _repository->findCommit("fac4c156026e8f20ac701585a07705b06b7f57b0");
    if(commit.isNull()) {
        throw TestException("Failed to find commit");
    }

    const QString tagName = "test-tag";

    const LightweightTag* createdTag = _repository->createLightweightTag(tagName, commit);
    if(createdTag == nullptr) {
        throw TestException("Failed to create tag");
    }

    const Tag* foundTag = _repository->findTag(tagName);
    if(foundTag == nullptr) {
        throw TestException("Failed to find tag after creation");
    }

    if(foundTag->isLightweight() == false) {
        throw TestException("Found tag is unexpected type");
    }

    LightweightTag concreteTag = foundTag->toLightweightTag();
    if(concreteTag.objectId() != createdTag->objectId()) {
        throw TestException("Found tag has different oid");
    }

    if(_repository->deleteTag(concreteTag.name()) == false) {
        throw TestException("Failed to delete tag we just created");
    }
}

void Examples::createAndDeleteAnnotatedTag()
{
    Commit commit = _repository->findCommit("fac4c156026e8f20ac701585a07705b06b7f57b0");
    if(commit.isNull()) {
        throw TestException("Failed to find commit");
    }

    const QString tagName = "test-annotated-tag";

    Signature signature("beavis", "beavis@butthead.com");
    const AnnotatedTag* createdTag = _repository->createAnnotatedTag(tagName, "This is my annotated tag", signature, commit);
    if(createdTag == nullptr) {
        throw TestException("Failed to create tag");
    }

    const Tag* foundTag = _repository->findTag(tagName);
    if(foundTag == nullptr) {
        throw TestException("Failed to find tag after creation");
    }

    if(foundTag->isAnnotated() == false) {
        throw TestException("Found tag is unexpected type");
    }

    AnnotatedTag concreteTag = foundTag->toAnnotatedTag();
    if(concreteTag.objectId() != createdTag->objectId()) {
        throw TestException("Found tag has different oid");
    }

    if(_repository->deleteTag(concreteTag.name()) == false) {
        throw TestException("Failed to delete tag we just created");
    }
}

void Examples::createAndDumpSomeDiffs()
{
    verifyNothingModified();

    ObjectId headObjectId = _repository->head().reference().objectId();
    Commit headCommit = _repository->findCommit(headObjectId);
    if(headCommit.isNull()) {
        throw TestException("Failed to find head commit");
    }

    QStringList filenames = {
        "subdir/testclass1.cpp",
        "subdir/testclass1.h",
    };

    for(const QString& filename : filenames) {
        modifyFile(filename);
    }

    StatusEntry::List modified = _repository->status().modified();
    if(modified.count() != filenames.count()) {
        throw TestException("Did not find the expected modifications");
    }

    CompareOptions compareOptions;
    compareOptions.setSimilarity(SimilarityOptions::none());
    DiffDelta::List deltas = _repository->diffDeltas(modified);
    for(const DiffDelta& delta : deltas) {
        _stdout << QString("old file: %1  new file: %2").arg(delta.oldFile().path()).arg(delta.newFile().path()) << Qt::endl;
        for(const DiffHunk& hunk : delta.hunks()) {
            _stdout << hunk.header();
            for(const DiffLine& line : hunk.lines()) {
                _stdout << line.origin() << line.content() << Qt::endl;
            }
        }
    }

    if(_repository->reset(headCommit, ResetHard) == false) {
        throw TestException("Failed to reset");
    }
}

void Examples::initializeRepo()
{
    if(_repository != nullptr) {
        return;
    }

    _repository = new Repository(_localPath);
    if(_repository->isNull()) {
        throw TestException("Failed to open repository");
    }
}

void Examples::modifyFile(const QString& relativePath)
{
    QString absolutePath = QString("%1/%2").arg(_repository->localPath()).arg(relativePath);
    QFile file(absolutePath);
    if(file.exists() == false) {
        throw TestException(QString("File %1 not found").arg(absolutePath));
    }

    if(file.open(QFile::Append) == false) {
        throw TestException(QString("Failed to open %1 for append").arg(absolutePath));
    }

    if(file.write(" ") == false) {
        throw TestException(QString("Failed to append to %1").arg(absolutePath));
    }
}

void ExampleProgressCallback::progressCallback(uint32_t receivedBytes, uint32_t receivedObjects, uint32_t totalObjects)
{
    qDebug() << QString("Clone progress: received %1 bytes  %2 of %3 objects").arg(receivedBytes).arg(receivedObjects).arg(totalObjects);
}
