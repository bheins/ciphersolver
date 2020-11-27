#include <cipher.h>
#include <cipherwordlineedit.h>
#include <QDropEvent>
#include <QMimeData>
#include <QListWidget>
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

TEST_F(CipherWordLineEditTest, DroppedWordFromWordListUpdatesTextInTextBox)
{
    CipherText->setPlainText("abcda");
    QList<CipherWordLineEdit*>editBoxes = sut.findChildren<CipherWordLineEdit*>();
    ASSERT_THAT(editBoxes.count(), Eq(1));

    CipherWordLineEdit* editBox = editBoxes.takeFirst();
    ASSERT_THAT(editBox, NotNull());

    QLineEdit *wordFilter(sut.findChild<QLineEdit*>("wordFilter"));
    ASSERT_THAT(wordFilter, NotNull());

    QListWidget* wordList(sut.findChild<QListWidget*>("wordSelection"));
    ASSERT_THAT(wordList, NotNull());

    wordFilter->setText("1...1");
    QTest::keyPress(wordFilter, Qt::EnterKeyReturn);
    ASSERT_THAT(wordList->count(), Gt(1));

    wordList->setCurrentRow(0);
//    QListWidgetItem* selectedWord= wordList->item(0);
//    QDragEnterEvent* enterEvent = new QDragEnterEvent(wordList->itemWidget(selectedWord)->pos(),Qt::MoveAction,);
    QMimeData* selectedWordData = new QMimeData();
    selectedWordData->setText("legal");
//    QDragMoveEvent* moveEvent = new QDragMoveEvent(wordList->itemWidget(selectedWord)->pos(),
//                                                   Qt::MoveAction,
//                                                   selectedWord->data(0),
//                                                   Qt::LeftButton);
//    QMetaObject::invokeMethod(wordList, "dragEvent", Q_ARG(QDragMoveEvent*, moveEvent))
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    buttons.setFlag(Qt::MouseButton::LeftButton);
    modifiers.setFlag(Qt::KeyboardModifier::NoModifier);
    QDropEvent* dropEvent = new  QDropEvent(editBox->pos(), Qt::MoveAction,selectedWordData,buttons, modifiers, QEvent::Type::Drop);
    QMetaObject::invokeMethod(editBox, "dropEvent", Q_ARG(QDropEvent*, dropEvent));
}
