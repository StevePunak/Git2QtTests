#ifndef TESTEXCEPTION_H
#define TESTEXCEPTION_H

#include <QException>
#include <QString>

class TestException : public QException
{
public:
    TestException(const QString& message, qint32 code = 0) :
        _message(message),
        _code(code) {}

    void raise() const override { throw *this; }
    TestException *clone() const override { return new TestException(*this); }

    QString message() const { return _message; }
    qint32 code() const { return _code; }

private:
    QString _message;
    qint32 _code;
};

#endif // TESTEXCEPTION_H
