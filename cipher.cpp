#include "cipher.h"
#include "ui_cipher.h"
#include "cipherwordlineedit.h"
#include <QDir>
#include <QTextDocument>
#include <QRegExp>
#include <QRegularExpression>
#include <QVector>
#include <QMouseEvent>
#include <QTextEdit>
#include <algorithm>
#include <QDebug>
#include <QInputDialog>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QFormLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QTextStream>
#include <QString>
#include <QListWidget>
#include <QObject>
#include <cipherobject.h>
#include <QHash>
#include <QMutexLocker>

cipher::cipher(QWidget *parent)
    : QMainWindow(parent)
    , CurrentSymbol("")
    , CurrentWord("")
    , TheCipher()
    , TheSolution()
    , ui(new Ui::CipherUI)
    , CipherMutex()
{
    ui->setupUi(this);
    load_dictionary();
}

cipher::~cipher()
{
    qDeleteAll(TheSolution);
}

void cipher::load_dictionary()
{
    QDir path(QCoreApplication::applicationDirPath().append("\\database"));
    QFile textFile(path.path().append("\\wordlist.txt"));
    QTextStream wordsTextStream(&textFile);

    if(textFile.exists() and textFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(not wordsTextStream.atEnd())
        {
            QString line = wordsTextStream.readLine();
            if(not line.isNull())
            {
                Dictionary.append(line);
            }
        }
        qDebug() << QString("Finished loading dictionary of %1 words.").arg(Dictionary.size());
    }
    else
    {
        qDebug() << QString("Error opening %1").arg(textFile.fileName());
    }

    QFile namesFile(path.path().append("\\Given-Names.txt"));
    QTextStream namesTextStream(&namesFile);

    if(namesFile.exists() and namesFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(not namesTextStream.atEnd())
        {
            QString line = namesTextStream.readLine();
            if(not line.isNull())
            {
                Dictionary.append(line);
            }
        }
        qDebug() << QString("Finished loading dictionary of %1 words.").arg(Dictionary.size());
    }
    else
    {
        qDebug() << QString("Error opening %1").arg(namesFile.fileName());
    }

}

void cipher::on_textEdit_textChanged()
{
    QMutexLocker locker(&CipherMutex);
    update_cipher();
    build_interactive_solver();
}

void cipher::update_cipher()
{
    QString newText(ui->textEdit->toPlainText());
    TheCipher.clear();

    qDeleteAll(TheSolution);
    TheSolution.clear();

    TheCipher.reserve(newText.size());//includes spaces, but will be large enough
    int index = 0;
    for(const auto& sentence : ui->textEdit->toPlainText().split("\n"))
    {
        int two_letter_words=std::count_if(sentence.begin(), sentence.end(), [](QString word){return word.size() == 2;});
        int word_count = sentence.split(" ").size();
        bool using_split_words=two_letter_words == word_count;

        QVector<QVector<QString> > theSentence;
        for(const auto& word : sentence.split(using_split_words ? "  ": " "))
        {
            QVector<QString> theWord;
            for(const auto& symbol: word)
            {
                theWord.push_back(symbol);
                TheSolution[index]=new cipherobj(symbol);
                ++index;
            }
            theSentence.push_back(theWord);
        }
        TheCipher.push_back(theSentence);
    }
}

void cipher::on_cipherwordlineedit_customContextMenuRequested(QPoint pos)
{
    CipherWordLineEdit* lineEdit(qobject_cast<CipherWordLineEdit*>(sender()));
    QString text(lineEdit->text());

    QString rePattern=text.replace("-",".").prepend("^").append("$");
    QRegExp searchFilter(rePattern, Qt::CaseInsensitive);
    QStringList filteredDatabaseList(Dictionary.filter(searchFilter));
    qDebug() << "wordFilter=" << rePattern << ".  FilteredDatabase resulted in " << filteredDatabaseList.size() << " items.";

    QPoint globalPos = lineEdit->mapToGlobal(pos);
    cipherobjectmenu solutionMenu;
    solutionMenu.move(globalPos);
    for(auto action : filteredDatabaseList)
    {
        solutionMenu.addAction(action);
    }
    solutionMenu.show();

    QAction* selectedItem = solutionMenu.exec(globalPos);
    if(nullptr not_eq selectedItem)
    {
        qDebug() << "Setting " << lineEdit->objectName() << " existing text " << lineEdit->text() << "=" << selectedItem->text();
        lineEdit->setText(selectedItem->text());
    }
}

void cipher::build_interactive_solver()
{
    QWidget* solver(ui->SolverVerticalLayout->findChild<QWidget*>("Solver"));
    if(nullptr not_eq solver)
    {
        QList<QWidget*> solverWidgets(solver->findChildren<QWidget*>(".+"));
        for(auto widget : solverWidgets)
        {
            ui->SolverVerticalLayout->removeWidget(widget);
            delete widget;
        }
        ui->SolverVerticalLayout->removeWidget(solver);
        delete solver;
    }
    solver = new QWidget(this);
    solver->setObjectName("Solver");

    int lineCount = 0;
    for(const auto& line : TheCipher)
    {
        QWidget *wordsGroupBox = new QWidget(solver);
        wordsGroupBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        QHBoxLayout *wordsLayout = new QHBoxLayout;
        wordsLayout->addStretch(1);

        int wordCount = 0;
        QWidget *untranslatedWordsGroupBox = new QWidget(solver);
        untranslatedWordsGroupBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        QHBoxLayout *untranslatedWordsLayout = new QHBoxLayout;
        untranslatedWordsLayout->addStretch(1);
        untranslatedWordsLayout->setSpacing(1);

        for(const auto& word: line)
        {
            QMap<int, cipherobj*> wordTranslation;
            int symbolCount=0;
            for(const auto& symbol: word)
            {
                wordTranslation.insert(symbolCount, find_cipher_by_untranslated_symbol(symbol));
                ++symbolCount;
            }
            QString lineEditTranslation;
            for(const auto cobj: wordTranslation)
            {
                lineEditTranslation.append(cobj->get_untranslated_symbol());
            }
            qDebug() << __PRETTY_FUNCTION__ << ":" << lineEditTranslation;
            CipherWordLineEdit *wordEditor = new CipherWordLineEdit(wordTranslation, lineCount, wordCount, wordsGroupBox);

            QLineEdit* untranslatedWordText = new QLineEdit(untranslatedWordsGroupBox);
            untranslatedWordText->setReadOnly(true);

            QString translation;
            QString untranslated;
            for(const auto& symbol: word)
            {
                translation.append(find_cipher_by_untranslated_symbol(symbol)->get_translated_symbol());
                untranslated.append(symbol);
            }
            wordEditor->setTextMargins(2,2,2,2);
            wordEditor->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(wordEditor, &QLineEdit::textChanged, this, &cipher::solver_textChanged, Qt::UniqueConnection);
            connect(wordEditor, &QLineEdit::customContextMenuRequested, this, &cipher::on_cipherwordlineedit_customContextMenuRequested);
            untranslatedWordText->setText(QString(untranslated));
            untranslatedWordText->setStyleSheet("* { background-color: rgba(0, 0, 0, 0); }");
            untranslatedWordText->setFrame(false);

            wordsLayout->addWidget(wordEditor);
            untranslatedWordsLayout->addWidget(untranslatedWordText);
            ++wordCount;
            wordsGroupBox->setLayout(wordsLayout);
            untranslatedWordsGroupBox->setLayout(untranslatedWordsLayout);
        }
        wordsGroupBox->show();
        untranslatedWordsGroupBox->show();
        ++lineCount;

        ui->SolverVerticalLayout->addWidget(wordsGroupBox);
        ui->SolverVerticalLayout->addWidget(untranslatedWordsGroupBox);
    }
    solver->show();
}

void cipher::solver_textChanged(const QString& text)
{
    QString lineEditor(sender()->objectName());
    QRegularExpression re("^solver_(?P<Line>\\d+)_(?P<Word>\\d+)$");
    QRegularExpressionMatch cipherMatcher = re.match(lineEditor);

    int line(cipherMatcher.captured("Line").toInt());
    int word(cipherMatcher.captured("Word").toInt());

    int symbol_index = 0;
    QVector<QString> cipherword = TheCipher[line][word];
    if(text.size() == cipherword.size())
    {
        for(const auto& symbol : text.split("", Qt::SkipEmptyParts))
        {
            cipherobj* untranslated_cipher_obj(find_cipher_by_untranslated_symbol(cipherword[symbol_index]));
            if(symbol == "")
            {
                qDebug() << QString("Clearing translation for %1=%2").arg(cipherword[symbol_index]).arg(symbol);
                untranslated_cipher_obj->add_translation(symbol);
            }
            else if(symbol not_eq "-")
            {
                qDebug() << lineEditor << ":" << "untranslated symbol=" << untranslated_cipher_obj->get_untranslated_symbol() << ", translated symbol=" << untranslated_cipher_obj->get_translated_symbol();
                if(untranslated_cipher_obj->get_translated_symbol() not_eq symbol)
                {
                    qDebug() << QString("Adding translation for %1=%2").arg(cipherword[symbol_index]).arg(symbol);
                    untranslated_cipher_obj->add_translation(symbol);
                    qDebug() << lineEditor << ":" << "untranslated symbol=" << untranslated_cipher_obj->get_untranslated_symbol() << ", translated symbol=" << untranslated_cipher_obj->get_translated_symbol();
                }
            }
            ++symbol_index;
        }
//        RemoveUnusedSymbolsFromSolution();
    }
    else
    {
        QString cipherString="";
        for (auto& symbol: cipherword)
        {
            cipherString.append(symbol);
        }
        qDebug() << QString("New Text (%1) for Cipher[%2][%3] (%4) does not match length (%5).").arg(text).arg(line).arg(word).arg(cipherString).arg(cipherword.size());
    }
}

void cipher::on_wordFilter_textEdited(const QString& text)
{
    qDebug() << "Word Search Filter=" << text;
    QString rePattern("^");
    QStringList repeatedSymbolsFilter(QString("123456789").split("", QString::SkipEmptyParts));
    QString unknownSymbolMatcher("(.)");
    //unknown letters should be matched as ".".
    //repeated unknown characters should be listed using numbers.
    //known characters obviously should be included.

    //eg banana would be matched by a regex using backreferencing and capture groups
    //if no translations are known.
    //user filter  = "......"
    //regex filter = "^(.)(.)(.)\2\3\2$"

    //if "a" is known with unknown "n" repeating and "b" not repeating.
    //user filter  = ".a1a1a"
    //regex filter = "^(.)a(.)a\2a$" if "a" is known with unknown "n" repeating

    //if "b" and "n" are known with unknown "a" repeating.
    //user filter  = "b1n1n1"
    //regex filter = "b12121"

    //if "b" and "n" are known with unknown "a" repeating.
    //"^b(.)n\1n\1$"
    //"^b(.)(.)\1\2\1$"

    int index = 0;
    QString reText(text);
    std::for_each(repeatedSymbolsFilter.constBegin(), repeatedSymbolsFilter.constEnd(), [&](QString repeatedSymbol)
    {
        reText.replace(repeatedSymbol, QString::number(repeatedSymbol.toInt() + 1));
    });

    QString lastSymbol;
    for(const QString& symbol : text)
    {
        if(not repeatedSymbolsFilter.contains(symbol))
        {
            rePattern.append(symbol);
        }
        else
        {
            if(index == text.indexOf(symbol))
            {
                rePattern.append(unknownSymbolMatcher);
            }
            else
            {
                rePattern.append("\\"+QString::number(symbol.toInt()));
            }
        }
        ++index;
        lastSymbol = symbol;
    }
    rePattern.append("$");
    QRegExp searchFilter(rePattern, Qt::CaseInsensitive);

    QStringList filteredDatabaseList(Dictionary.filter(searchFilter));
    ui->wordSelection->clear();
    qDebug() << "wordFilter=" << rePattern << ".  FilteredDatabase resulted in " << filteredDatabaseList.size() << " items.";
    ui->wordSelection->insertItems(0, filteredDatabaseList);
}

void cipher::on_wordSelection_itemClicked(QListWidgetItem *selection)
{
    qDebug() << "User selected " << selection->text();
}

cipherobj* cipher::find_cipher_by_untranslated_symbol(const QString& symbol)
{
    auto it = std::find_if(TheSolution.constBegin(), TheSolution.constEnd(), [&](cipherobj* cipher)
    {
            bool match = (symbol == cipher->get_untranslated_symbol());
            qDebug() << __PRETTY_FUNCTION__ << ": " << symbol << (match ? "==" : "!=") << cipher->get_untranslated_symbol();
            return match;
             }
             );
    cipherobj* cobj(it == TheSolution.constEnd() ? nullptr : *it);
    return cobj;
}

cipherobj* cipher::find_cipher_by_translated_symbol(const QString& symbol)
{
    auto it = std::find_if(TheSolution.constBegin(), TheSolution.constEnd(), [&](cipherobj* cipher)
    {
            bool match = (symbol == cipher->get_translated_symbol());
            qDebug() << __PRETTY_FUNCTION__ << ": " << symbol << (match ? "==" : "!=") << cipher->get_translated_symbol();
            return match;
             }
             );
    cipherobj* cobj(it == TheSolution.constEnd() ? nullptr : *it);

    return cobj;
}
