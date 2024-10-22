#ifndef REPOCOPY_H
#define REPOCOPY_H
#include <git2qt.h>
#include <Kanoop/utility/loggingbaseclass.h>

class RepoCopy : public LoggingBaseClass
{
public:
    RepoCopy(const QString& fromPath, const QString& toPath, const QDateTime& firstNewCommitTime);

    bool execute();

private:
    void openRepositories();

    void copyCommits();

    void createInitialCommit(const GIT::GraphedCommit& commit);
    void ensureBranch(const GIT::GraphedCommit& commit);
    void emplaceTreeToFilesystem(GIT::Tree& tree);
    void writeBlob(GIT::TreeEntry& entry);
    void writeTree(GIT::TreeEntry& entry);
    void createTargetCommit(const GIT::GraphedCommit& commit);

    QString _fromPath;
    QString _toPath;
    QDateTime _firstOldTime;
    QDateTime _firstNewTime;

    GIT::Repository* _fromRepo = nullptr;
    GIT::Repository* _toRepo = nullptr;
    GIT::GraphedCommit::List _fromCommits;
};

#endif // REPOCOPY_H
