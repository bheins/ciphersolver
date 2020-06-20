#include <cipher.h>
#include <cipherwordlineedit.h>
#include <QPlainTextEdit>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <qtest.h>

using namespace testing;

class CipherWordLineEditTest: public testing::Test
{
  public:
    CipherWordLineEditTest()
        : sut()
        , CipherText(sut.findChild<QPlainTextEdit*>("Cipher"))
    {}
    virtual ~CipherWordLineEditTest() = default;

    virtual void SetUp()
    {
        ASSERT_THAT(CipherText, NotNull());
    }

protected:
    cipher sut;
    QPlainTextEdit* CipherText;
};

TEST_F(CipherWordLineEditTest, EditBoxesAddedForEachWord)
{
    CipherText->setPlainText("a bb ccc");

    QList<CipherWordLineEdit*> editBoxes = sut.findChildren<CipherWordLineEdit*>();
    ASSERT_THAT(editBoxes.count(), Eq(3));

    CipherWordLineEdit* editBox = editBoxes.takeFirst();
    ASSERT_THAT(editBox, NotNull());
    EXPECT_STREQ("solver_0_0", editBox->objectName().toLatin1().constData());

    editBox = editBoxes.takeFirst();
    EXPECT_STREQ("solver_0_1", editBox->objectName().toLatin1().constData());

    editBox = editBoxes.takeFirst();
    EXPECT_STREQ("solver_0_2", editBox->objectName().toLatin1().constData());
}

TEST_F(CipherWordLineEditTest, EditBoxesAddedForEachLine)
{
    CipherText->setPlainText("a\nbb\nccc");

    QList<CipherWordLineEdit*>editBoxes = sut.findChildren<CipherWordLineEdit*>();
    ASSERT_THAT(editBoxes.count(), Eq(3));

    CipherWordLineEdit* editBox = editBoxes.takeFirst();
    ASSERT_THAT(editBox, NotNull());
    EXPECT_STREQ("solver_0_0", editBox->objectName().toLatin1().constData());

    editBox = editBoxes.takeFirst();
    EXPECT_STREQ("solver_1_0", editBox->objectName().toLatin1().constData());

    editBox = editBoxes.takeFirst();
    EXPECT_STREQ("solver_2_0", editBox->objectName().toLatin1().constData());
}

TEST_F(CipherWordLineEditTest, EditBoxesShowDashForUntranslatedLetters)
{
    CipherText->setPlainText("a bb ccc");

    QList<CipherWordLineEdit*>editBoxes = sut.findChildren<CipherWordLineEdit*>();
    ASSERT_THAT(editBoxes.count(), Eq(3));

    CipherWordLineEdit* editBox = editBoxes.takeFirst();
    ASSERT_THAT(editBox, NotNull());
    EXPECT_STREQ("-", editBox->text().toLatin1().constData());

    editBox = editBoxes.takeFirst();
    ASSERT_THAT(editBox, NotNull());
    EXPECT_STREQ("--", editBox->text().toLatin1().constData());

    editBox = editBoxes.takeFirst();
    ASSERT_THAT(editBox, NotNull());
    EXPECT_STREQ("---", editBox->text().toLatin1().constData());
}

TEST_F(CipherWordLineEditTest, AddingTranslationForSymbolUpdatesAllSymbolsInOtherEditBoxes)
{
    CipherText->setPlainText("a bab aca");

    QList<CipherWordLineEdit*>editBoxes = sut.findChildren<CipherWordLineEdit*>();
    ASSERT_THAT(editBoxes.count(), Eq(3));

    CipherWordLineEdit* editBoxA = editBoxes.takeFirst();
    ASSERT_THAT(editBoxA, NotNull());
    EXPECT_STREQ("-", editBoxA->text().toLatin1().constData());

    CipherWordLineEdit* editBoxB = editBoxes.takeFirst();
    ASSERT_THAT(editBoxB, NotNull());
    EXPECT_STREQ("---", editBoxB->text().toLatin1().constData());

    CipherWordLineEdit* editBoxC = editBoxes.takeFirst();
    ASSERT_THAT(editBoxC, NotNull());
    EXPECT_STREQ("---", editBoxC->text().toLatin1().constData());

    editBoxA->setText("I");
    EXPECT_STREQ("I", editBoxA->text().toLatin1().constData());
    EXPECT_STREQ("-I-", editBoxB->text().toLatin1().constData());
    EXPECT_STREQ("I-I", editBoxC->text().toLatin1().constData());
}
