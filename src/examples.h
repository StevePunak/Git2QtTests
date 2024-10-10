#ifndef EXAMPLES_H
#define EXAMPLES_H
#include <git2qt.h>

class Examples
{
public:
    Examples(const QString& localPath);

    void autoTest();

    static void cloneRepo(const QString& remoteUrl, const QString& localPath);
    void listBranches();
    void listIndexes();
    void verifyNothingModified();
    void stageAndUnstageSomeFiles();
    void unstageAllFiles();
    void commitSomeChangesAndResetHard();
    void createAndDeleteLightweightTag();
    void createAndDeleteAnnotatedTag();
    void createAndDumpSomeDiffs();

    QString errorText() const { return _errorText; }

private:
    void initializeRepo();
    void modifyFile(const QString& relativePath);

    QString _remoteUrl;
    QString _localPath;
    QTextStream _stdout;

    GIT::Repository* _repository;

    QString _errorText;
};

class ExampleCredentialResolver : public GIT::AbstractCredentialResolver
{
public:
    virtual QString getUsername() const override { return "libgit2qt_tester"; }
    virtual QString getPassword() const override { return "libgit2qt_tester"; }
};

class ExampleProgressCallback : public GIT::ProgressCallback
{
public:
    virtual void progressCallback(uint32_t receivedBytes, uint32_t receivedObjects, uint32_t totalObjects) override;
};

#endif // EXAMPLES_H
