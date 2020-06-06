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

bool cipher::is_untranslated(const QString& symbol)
{
    return (symbol=="-");
}

void cipher::on_cipherwordlineedit_customContextMenuRequested(QPoint pos)
{
    CipherWordLineEdit* lineEdit(qobject_cast<CipherWordLineEdit*>(sender()));
    QString lineEditor(sender()->objectName());
    QRegularExpression re("^solver_(?P<Line>\\d+)_(?P<Word>\\d+)$");
    QRegularExpressionMatch cipherMatcher = re.match(lineEditor);

    int line(cipherMatcher.captured("Line").toInt());
    int word(cipherMatcher.captured("Word").toInt());
    QVector<QString> cipherword = TheCipher[line][word];

    QString reSearchPattern="^";
    QString text(lineEdit->text());
    int symbol_index=0;
    int matched_repeating_letter_index=0;
    for(auto symbol : lineEdit->text().split("", Qt::SkipEmptyParts))
    {
        QVector<int> matchingSymbols(find_all_matching_untranslated_symbols(line, word, symbol_index));
        if(is_untranslated(symbol))
        {
            if(matchingSymbols.size() == 1)
            {
                text.replace(symbol_index, 1, ".");
            }
            else if(matchingSymbols.size() > 1)
            {
                for(auto repeat_index : matchingSymbols)
                {
                    if(text[repeat_index] not_eq matched_repeating_letter_index)
                    {
                        text.replace(repeat_index, 1, QString::number(matched_repeating_letter_index));
                    }
                }
                ++matched_repeating_letter_index;
            }
        }
        else
        {
            //match translated symbol exactly
        }
        ++symbol_index;
    }
    QString rePattern=generate_regex_search_string_from_pattern(text);
    qDebug() << __PRETTY_FUNCTION__ << ": searching for matching regex on database with " << rePattern;

    QRegExp searchFilter(rePattern, Qt::CaseInsensitive);
    QStringList filteredDatabaseList(Dictionary.filter(searchFilter));

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
            qDebug() << "removing widget " << widget->objectName();
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
        QWidget *lineGroupBox = new QWidget(solver);
        QHBoxLayout *linesLayout = new QHBoxLayout;
        lineGroupBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        linesLayout->setSpacing(1);

        int wordCount = 0;
        for(const auto& word: line)
        {
            QWidget *wordGroupBox = new QWidget(solver);
            wordGroupBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            QVBoxLayout *wordsLayout = new QVBoxLayout;
            wordsLayout->setSpacing(1);

            QMap<int, cipherobj*> wordTranslation;
            int symbolCount=0;
            for(const auto& symbol: word)
            {
                wordTranslation.insert(symbolCount, find_cipher_by_untranslated_symbol(symbol));
                ++symbolCount;
            }

            CipherWordLineEdit *wordEditor = new CipherWordLineEdit(wordTranslation, lineCount, wordCount, wordGroupBox);
            wordEditor->setTextMargins(2,2,2,2);
            wordEditor->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(wordEditor, &QLineEdit::textChanged, this, &cipher::solver_textChanged, Qt::UniqueConnection);
            connect(wordEditor, &QLineEdit::customContextMenuRequested, this, &cipher::on_cipherwordlineedit_customContextMenuRequested);

            QLineEdit* untranslatedWordText = new QLineEdit(wordGroupBox);
            untranslatedWordText->setReadOnly(true);

            QString untranslated;
            for(const auto& symbol: word)
            {
                untranslated.append(symbol);
            }
            untranslatedWordText->setText(QString(untranslated));
            untranslatedWordText->setStyleSheet("* { background-color: rgba(0, 0, 0, 0); }");
            untranslatedWordText->setFrame(false);

            wordsLayout->addWidget(wordEditor);
            wordGroupBox->setLayout(wordsLayout);
            wordGroupBox->show();

            wordsLayout->addWidget(untranslatedWordText);
            linesLayout->addWidget(wordGroupBox);
            ++wordCount;
        }
        lineGroupBox->setLayout(linesLayout);
        lineGroupBox->show();
        ++lineCount;

        ui->SolverVerticalLayout->addWidget(lineGroupBox);
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
                if(untranslated_cipher_obj->get_translated_symbol() not_eq symbol)
                {
                    qDebug() << QString("Adding translation for %1=%2").arg(cipherword[symbol_index]).arg(symbol);
                    untranslated_cipher_obj->add_translation(symbol);
                }
            }
            ++symbol_index;
        }
    }
    else
    {
        QString cipherString="";
        for (auto& symbol: cipherword)
        {
            cipherString.append(symbol);
        }
    }
}

QString cipher::generate_regex_search_string_from_pattern(const QString& pattern)
{
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

    QStringList repeatedSymbolsFilter(QString("123456789").split("", QString::SkipEmptyParts));
    QString unknownSymbolMatcher("(.)");

    int index = 0;
    QString rePattern("^");
    QString reText(pattern);
    std::for_each(repeatedSymbolsFilter.constBegin(), repeatedSymbolsFilter.constEnd(), [&](QString repeatedSymbol)
    {
        reText.replace(repeatedSymbol, QString::number(repeatedSymbol.toInt() + 1));
    });

    QString lastSymbol;
    for(const QString& symbol : pattern)
    {
        if(not repeatedSymbolsFilter.contains(symbol))
        {
            rePattern.append(symbol);
        }
        else
        {
            if(index == pattern.indexOf(symbol))
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

    return rePattern;
}

void cipher::on_wordFilter_textEdited(const QString& text)
{
    QString rePattern(generate_regex_search_string_from_pattern(text));
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
            return match;
             }
             );
    cipherobj* cobj(it == TheSolution.constEnd() ? nullptr : *it);
    return cobj;
}

QVector<int> cipher::find_all_matching_untranslated_symbols(const int line, const int word, const int symbol_index)
{
    QVector<int> matchingSymbols;
    int index=0;
    QString matching_symbol(TheCipher[line][word][symbol_index]);
    std::for_each(TheCipher[line][word].constBegin(), TheCipher[line][word].constEnd(), [&](QString untranslated_symbol)
    {
        if(matching_symbol == untranslated_symbol)
        {
            matchingSymbols.append(index);
        }
        ++index;
    });

    return matchingSymbols;
}

cipherobj* cipher::find_cipher_by_translated_symbol(const QString& symbol)
{
    auto it = std::find_if(TheSolution.constBegin(), TheSolution.constEnd(), [&](cipherobj* cipher)
    {
            bool match = (symbol == cipher->get_translated_symbol());
            return match;
             }
             );
    cipherobj* cobj(it == TheSolution.constEnd() ? nullptr : *it);

    return cobj;
}
