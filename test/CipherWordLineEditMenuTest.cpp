#include <cipher.h>
#include <cipherwordlineedit.h>
#include <QAction>
#include <QPlainTextEdit>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <qtest.h>

using namespace testing;

class CipherWordLineEditMenuTest: public testing::Test
{
  public:
    CipherWordLineEditMenuTest()
        : sut()
        , CipherText(sut.findChild<QPlainTextEdit*>("Cipher"))
    {}
    virtual ~CipherWordLineEditMenuTest() = default;

    virtual void SetUp()
    {
        ASSERT_THAT(CipherText, NotNull());
    }

    QWidget* FindUnparentedWidget(const QString & name)
    {
        QWidget* widget = nullptr;
        const QWidgetList topLevelWidgets = QApplication::topLevelWidgets();

        auto foundWidget=std::find_if(topLevelWidgets.constBegin(), topLevelWidgets.constEnd(), [&](const QWidget* widget)
        {
            qDebug() << "objectName=" << widget->objectName();
            return widget->objectName() == name;
        });

        if(foundWidget not_eq topLevelWidgets.constEnd())
        {
            widget=*foundWidget;
        }

        return widget;
    }

protected:
    cipher sut;
    QPlainTextEdit* CipherText;
};

TEST_F(CipherWordLineEditMenuTest, ContextMenuForEditBoxShowsWordsMatchingPattern)
{
    CipherText->setPlainText("a bab aca");
    QList<CipherWordLineEdit*>editBoxes = sut.findChildren<CipherWordLineEdit*>();
    ASSERT_THAT(editBoxes.count(), Eq(3));

    CipherWordLineEdit* editBox = editBoxes.takeFirst();
    ASSERT_THAT(editBox, NotNull());

    QMetaObject::invokeMethod(editBox, "customContextMenuRequested", Q_ARG(QPoint, editBox->pos()));
    cipherobjectmenu* cipherMenu = sut.findChild<cipherobjectmenu*>();
    ASSERT_THAT(cipherMenu, NotNull());
    const QList<QAction*> words = cipherMenu->actions();
    ASSERT_EQ(2, words.count());
    EXPECT_STREQ("a", words.at(0)->text().toLatin1().constData());
    EXPECT_STREQ("i", words.at(1)->text().toLatin1().constData());
    QMetaObject::invokeMethod(words.at(1), "triggered", Q_ARG(bool, false));
}

TEST_F(CipherWordLineEditMenuTest, CipherTextUpdatedWithSolutionWhenWordSelected)
{
    CipherText->setPlainText("a bab aca");
    QList<CipherWordLineEdit*>editBoxes = sut.findChildren<CipherWordLineEdit*>();
    ASSERT_THAT(editBoxes.count(), Eq(3));

    CipherWordLineEdit* editBox = editBoxes.takeFirst();
    ASSERT_THAT(editBox, NotNull());

    QMetaObject::invokeMethod(editBox, "customContextMenuRequested", Q_ARG(QPoint, editBox->pos()));
    cipherobjectmenu* cipherMenu = sut.findChild<cipherobjectmenu*>();
    ASSERT_THAT(cipherMenu, NotNull());
    const QList<QAction*> words = cipherMenu->actions();
    ASSERT_EQ(2, words.count());
    EXPECT_STREQ("a", words.at(0)->text().toLatin1().constData());
    EXPECT_STREQ("i", words.at(1)->text().toLatin1().constData());

    QMetaObject::invokeMethod(words.at(1), "triggered", Q_ARG(bool, false));
    EXPECT_STREQ("i", editBox->text().toLatin1().constData());
    EXPECT_STREQ("-i-", editBoxes.takeFirst()->text().toLatin1().constData());
    EXPECT_STREQ("i-i", editBoxes.takeFirst()->text().toLatin1().constData());
}
