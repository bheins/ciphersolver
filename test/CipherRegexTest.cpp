#include <cipher.h>
#include <QtTest/QtTest>

#include "CipherRegexTest.moc"
class CipherRegexTest: public QObject
{
    Q_OBJECT
  public:
    CipherRegexTest()
        : sut()
    {}
    ~CipherRegexTest() = default;

private slots:
    void MonoAlphabeticSearchDoesntReturnIncorrectResults();

private:
    cipher sut;
};

void CipherRegexTest::MonoAlphabeticSearchDoesntReturnIncorrectResults()
{
    sut.show();
    QVERIFY(sut.isVisible());
}

QTEST_MAIN(CipherRegexTest)
