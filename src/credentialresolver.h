#ifndef CREDENTIALRESOLVER_H
#define CREDENTIALRESOLVER_H
#include <git2qt.h>

class CredentialResolver : public GIT::AbstractCredentialResolver
{
public:
    CredentialResolver(const QString& username, const QString& password, const QString& publicKeyFile, const QString& privateKeyFile) :
        _username(username), _password(password), _publicKeyFile(publicKeyFile), _privateKeyFile(privateKeyFile) {}

    virtual QString getUsername() const override { return _username; }
    virtual QString getPassword() const override { return _password; }
    virtual QString getPublicKeyFilename() const override { return _publicKeyFile; }
    virtual QString getPrivateKeyFilename() const override { return _privateKeyFile; }

private:
    QString _username;
    QString _password;
    QString _publicKeyFile;
    QString _privateKeyFile;
};


#endif // CREDENTIALRESOLVER_H
