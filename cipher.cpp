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

void cipherobj::add_translation(QString known)
{
    translated_symbol = known;
    translated = true;
}

QString cipherobj::get_translated_symbol() const
{
    return translated_symbol;
}

void cipherobj::set_untranslated_symbol(const QString& symbol)
{
    untranslated_symbol = symbol;
}

QString cipherobj::get_untranslated_symbol() const
{
    return untranslated_symbol;
}

bool cipherobj::is_translated() const
{
    return translated;
}


cipher::cipher(QWidget *parent)
    : QMainWindow(parent)
    , CurrentSymbol("")
    , CurrentWord("")
    , TheCipher()
    , TheSolution()
    , ui(new Ui::CipherUI)
    , TheMenu()
{
    ui->setupUi(this);
    ui->textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    load_dictionary();

    //connectSlotsByName takes care of these for us (slot name must be written as on_object_signal)
    //connect(ui->textEdit, &QWidget::customContextMenuRequested, this, &cipher::on_textEdit_customContextMenuRequested);
    //connect(ui->textEdit, &QTextEdit::textChanged, this, &cipher::on_textEdit_textChanged);
    //connect(ui->wordFilter, &QLineEdit::textEdited, this, &cipher::on_wordFilter_textEdited);
    //connect(ui->wordSelection, &QListWidget::itemClicked, this, &cipher::on_wordSelection_itemClicked);
}

cipher::~cipher() = default;

void cipher::reapply_highlighting_rules()
{

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
    parse_text();
    build_interactive_solver();
}

void cipher::UpdateCipher()
{
    QString newText(ui->textEdit->toPlainText());
    TheCipher.clear();
    TheCipher.reserve(newText.size());//includes spaces, but will be large enough
    for(const auto& sentence : ui->textEdit->toPlainText().split("\n"))
    {
        int two_letter_words=std::count_if(sentence.begin(), sentence.end(), [](QString word){return word.size() == 2;});
        int word_count = sentence.split(" ").size();
        bool using_split_words=two_letter_words == word_count;

        QVector<QVector<cipherobj> > theSentence;
        for(const auto& word : sentence.split(using_split_words ? "  ": " "))
        {
            QVector<cipherobj> theWord;
            for(const auto& symbol: word)
            {
                cipherobj theCipherObj;
                theCipherObj.set_untranslated_symbol(symbol);
                theWord.push_back(theCipherObj);
            }
            theSentence.push_back(theWord);
        }
        TheCipher.push_back(theSentence);
    }
}

void cipher::UpdateSolution()
{
    QStringList cipherText;
    QStringList solutionText;
    for(const auto& line : TheCipher)
    {
        for(const auto& word : line)
        {
            QString cipherWord;
            QString solutionWord;
            for(const auto& symbolObj : word)
            {
                const auto& symbol = symbolObj.get_untranslated_symbol();
                if(not TheSolution.contains(symbol))
                {
                    TheSolution.insert(symbol, QString("_"));
                }
                cipherWord.append(symbol);
                solutionWord.append(TheSolution[symbol]);
            }
            cipherText.append(cipherWord);
            solutionText.append(solutionWord);
        }
        solutionText.append(" ");
    }
    qDebug() << "Cipher=" << cipherText.join(" ");
    qDebug() << "Solution=" << solutionText.join(" ");
}

void cipher::RemoveUnusedSymbolsFromSolution()
{
    for(const auto& symbol : TheSolution.keys())
    {
        if(not CipherContainsSymbol(symbol))
        {
            TheSolution.remove(symbol);
        }
    }
}

void cipher::parse_text()
{
    UpdateCipher();
    UpdateSolution();
    RemoveUnusedSymbolsFromSolution();
}

bool cipher::CipherContainsSymbol(const QString& symbol)
{
    bool found=false;
    for(const auto& line : TheCipher)
    {
        for(const auto& word : line)
        {
            if(std::any_of(word.constBegin(), word.constEnd(), [&](cipherobj obj) {return obj.get_untranslated_symbol() == symbol;}))
            {
                found=true;
                break;
            }
        }
    }
    return found;
}

void cipher::on_textEdit_customContextMenuRequested(QPoint pos)
{
    QTextCursor textEditCursor(ui->textEdit->textCursor());
    textEditCursor.select(QTextCursor::WordUnderCursor);
    CurrentWord = textEditCursor.selection().toPlainText();
    qDebug() << " Selected word=" << CurrentWord << " Column=" << textEditCursor.columnNumber();

    QPoint globalPos = ui->textEdit->mapToGlobal(pos);
    TheMenu.move(globalPos);
    TheMenu.show();

    QAction* selectedItem = TheMenu.exec(globalPos);
    if(nullptr not_eq selectedItem)
    {
        qDebug() << selectedItem->text();
        if(selectedItem == TheMenu.actionToggleTranslatable)
        {
            qDebug() << "Toggle translation allowed for block" << ui->textEdit->textCursor().blockNumber();
        }
    }
}

void cipher::build_interactive_solver()
{
    for(QWidget* box : ui->solver->findChildren<QWidget*>(""))
    {
        delete box;
        box = nullptr;
    }

    int lineCount = 0;
    for(const auto& line : TheCipher)
    {
        QWidget *wordsGroupBox = new QWidget(ui->solver);
        wordsGroupBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        QHBoxLayout *wordsLayout = new QHBoxLayout;
        wordsLayout->addStretch(1);
        wordsLayout->addSpacerItem(new QSpacerItem(10,0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

        int wordCount = 0;
        for(const auto& word: line)
        {
            CipherWordLineEdit *wordEditor = new CipherWordLineEdit(wordsGroupBox);
            wordEditor->setObjectName(QString("solver_%1_%2").arg(lineCount).arg(wordCount));

            QString translation;
            for(auto& cipherObj: word)
            {
                translation.append(cipherObj.get_translated_symbol());
            }
            wordEditor->setText(translation);
            wordEditor->setTextMargins(5,5,5,5);
            connect(wordEditor, &QLineEdit::textChanged, this, &cipher::solver_textChanged);

            wordsLayout->addWidget(wordEditor);
            //wordsLayout->addSpacerItem(new QSpacerItem(10,10, QSizePolicy::Fixed, QSizePolicy::Fixed));
            ++wordCount;
            wordsGroupBox->setLayout(wordsLayout);
        }
        wordsGroupBox->show();
        ++lineCount;
        ui->SolverVerticalLayout->addWidget(wordsGroupBox);
    }

    //ui->solver->resize(ui->textEdit->sizeHint());
    //qDebug() << "Solver sizeHint=(" << ui->solver->sizeHint().width() << ", " << ui->solver->sizeHint().height() << ")";
    ui->solver->show();
}

void cipher::solver_textChanged(const QString& text)
{
    QString lineEditor(sender()->objectName());
    QRegularExpression re("^solver_(?P<Line>\\d+)_(?P<Word>\\d+)$");
    QRegularExpressionMatch cipherMatcher = re.match(lineEditor);
    qDebug() << lineEditor;
    int line(cipherMatcher.captured("Line").toInt());
    int word(cipherMatcher.captured("Word").toInt());

    int symbol_index = 0;
    if(text.size() == TheCipher[line][word].size())
    {
        for(const auto& symbol : text.split("", QString::SkipEmptyParts))
        {
            bool translated(symbol not_eq "_");
            if(translated)
            {
                TheCipher[line][word][symbol_index].add_translation(symbol);
            }

            ++symbol_index;
        }
        UpdateSolution();
        RemoveUnusedSymbolsFromSolution();
    }
    else
    {
        qDebug() << QString("New Text (%1) for Cipher[%2][%3] does not match length.").arg(text).arg(line).arg(word);
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
