#include "repocopy.h"

#include <Kanoop/commonexception.h>
#include <Kanoop/fileutil.h>
#include <Kanoop/pathutil.h>

using namespace GIT;

RepoCopy::RepoCopy(const QString& fromPath, const QString& toPath, const QDateTime& firstNewCommitTime) :
    LoggingBaseClass("repocopy"),
    _fromPath(fromPath), _toPath(toPath),
    _firstNewTime(firstNewCommitTime)
{
}

bool RepoCopy::execute()
{
    bool result = false;
    try
    {
        openRepositories();
        copyCommits();

        result = true;
    }
    catch(const CommonException& e)
    {
        result = false;
        logText(LVL_ERROR, e.message());
    }
    return result;
}

void RepoCopy::openRepositories()
{
    _fromRepo = new Repository(_fromPath);
    if(_fromRepo == nullptr) {
        throw CommonException(QString("Failed to open source repo at %1").arg(_fromPath));
    }

    QDir targetDir(_toPath);
    if(targetDir.exists() && targetDir.removeRecursively() == false) {
        throw CommonException("Failed to remove dest");
    }

    _toRepo = Commands::createRepository(_toPath);
    if(_toRepo == nullptr) {
        throw CommonException(QString("Failed to open destination repo at %1 (%2)").arg(_toPath).arg(Commands::lastErrorText()));
    }

    _fromCommits = _fromRepo->commitGraph();
    std::reverse(_fromCommits.begin(), _fromCommits.end());
    _firstOldTime = _fromCommits.at(0).author().timestamp();
}

void RepoCopy::copyCommits()
{
    createInitialCommit(_fromCommits.at(0));

    for(int i = 1;i < _fromCommits.count();i++) {
        GraphedCommit fromCommit = _fromCommits.at(i);
        ensureBranch(fromCommit);

        if(fromCommit.isMerge() == false) {
            Tree tree = fromCommit.tree();
            emplaceTreeToFilesystem(tree);
        }

        createTargetCommit(fromCommit);
    }
}

void RepoCopy::createInitialCommit(const GIT::GraphedCommit& commit)
{
    Tree tree = commit.tree();
    emplaceTreeToFilesystem(tree);

    createTargetCommit(commit);
    Branch head = _toRepo->head();
    Commit headCommit = _toRepo->headCommit();
}

void RepoCopy::ensureBranch(const GIT::GraphedCommit& commit)
{
    if(_toRepo->branches().findLocalBranch(commit.friendlyBranchName()).isValid() == false) {
        if(_toRepo->createBranch(commit.friendlyBranchName(), true).isValid() == false) {
            throw CommonException(QString("Failed to create branch %1 (%2)").arg(commit.friendlyBranchName()).arg(_toRepo->errorText()));
        }
    }
    if(_toRepo->checkoutLocalBranch(commit.friendlyBranchName()) == false) {
        throw CommonException(QString("Failed to checkout branch %1 (%2)").arg(commit.friendlyBranchName()).arg(_toRepo->errorText()));
    }
}

void RepoCopy::emplaceTreeToFilesystem(GIT::Tree& tree)
{
    for(TreeEntry& entry : tree.entries()) {
        logText(LVL_DEBUG, QString("entry: type: %1  path: %2").arg(getObjectTypeString(entry.entryType())).arg(entry.path()));
        switch(entry.targetType()) {
        case ObjectTypeBlob:
            writeBlob(entry);
            break;
        case ObjectTypeTree:
            writeTree(entry);
            break;
        default:
            throw CommonException(QString("Unhandled tree entry type: %1").arg(getObjectTypeString(entry.targetType())));
        }
    }
}

void RepoCopy::writeBlob(GIT::TreeEntry& entry)
{
    const Blob* blob = dynamic_cast<const Blob*>(entry.target());
    if(blob == nullptr) {
        throw CommonException("Failed to cast object to blob");
    }
    Blob deref = *blob;
    QString path = PathUtil::combine(_toPath, entry.path());
    QFileInfo fileInfo(path);
    QDir dir(fileInfo.absolutePath());
    if(dir.exists() == false) {
        if(dir.mkpath(".") == false) {
            throw CommonException(QString("Failed to create path %1").arg(fileInfo.absolutePath()));
        }
    }
    if(FileUtil::writeAllBytes(path, deref.rawData()) == false) {
        throw CommonException(QString("Failed to write %1").arg(path));
    }
}

void RepoCopy::writeTree(GIT::TreeEntry& entry)
{
    const Tree* tree = dynamic_cast<const Tree*>(entry.target());
    if(tree == nullptr) {
        throw CommonException("Failed to cast object to tree");
    }
    Tree deref = *tree;
    emplaceTreeToFilesystem(deref);
}

void RepoCopy::createTargetCommit(const GIT::GraphedCommit& commit)
{
    _toRepo->stage("*");
    Signature author = commit.author();
    Signature committer = commit.committer();

    TimeSpan diffTime = TimeSpan::absDiff(author.timestamp(), _firstOldTime);
    QDateTime timestamp = _firstNewTime.addSecs(diffTime.totalSeconds());
    author.setTimestamp(timestamp);
    committer.setTimestamp(timestamp);

    if(commit.isMerge() == false) {
        if(_toRepo->commit(commit.message(), author, committer).isValid() == false) {
            throw CommonException(QString("Failed to create target commit for %1").arg(commit.message().trimmed()));
        }
    }
    else {
        // if(_toRepo->checkoutLocalBranch(commit.friendlyBranchName()) == false) {
        //     throw CommonException("Failed to checkout a branch");
        // }
        GraphedCommit::List toCommits = _toRepo->commitGraph().reverse();
        GraphedCommit fromRepoMergeFromCommit = _fromCommits.findCommit(commit.mergeFrom());
        int mergeFromIndex = _fromCommits.indexOf(fromRepoMergeFromCommit);

        GraphedCommit mergeFromCommit = toCommits.at(mergeFromIndex);
        if(mergeFromCommit.isValid() == false) {
            throw CommonException("Merge commits not found");
        }

        MergeOptions options;
        options.setFastForwardStrategy(NoFastForward);

        QString message = QString("Merge branch '%1' into %2").arg(mergeFromCommit.friendlyBranchName()).arg(commit.friendlyBranchName());
        MergeResult result = _toRepo->merge(mergeFromCommit, author, message, options);
        if(result.isValid() == false) {
            throw CommonException("Failed to create merge");
        }
    }
}
