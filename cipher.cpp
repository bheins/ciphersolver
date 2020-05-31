#include "cipher.h"
#include "ui_cipher.h"
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
    connect(ui->textEdit, &QWidget::customContextMenuRequested, this, &cipher::on_customContextMenuRequested);
    connect(ui->textEdit, &QTextEdit::textChanged, this, &cipher::on_textChanged);
}

cipher::~cipher()
{
}

void cipher::reapply_highlighting_rules()
{

}

void cipher::load_dictionary()
{
    QDir path(QCoreApplication::applicationDirPath().append("database"));
    QFile database(path.dirName().append("\\wordlist.txt"));
            if (database.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                //NOOP
            }
}

void cipher::on_textChanged()
{
    parse_text();
    build_interactive_solver();
}

void cipher::UpdateCipher()
{
    for(const auto& sentence : ui->textEdit->toPlainText().split("\n"))
    {
        int two_letter_words=std::count_if(sentence.begin(), sentence.end(), [](QString word){return word.size() == 2;});
        int word_count = sentence.split(" ").size();
        bool using_split_words=two_letter_words == word_count;

        QVector<QVector<cipherobj> > theSentence;
        for(const auto& word : sentence.split(using_split_words ? "  ": " "))
        {
            QVector<cipherobj> theWord;
            for(const auto& symbol: qAsConst(word))
            {
                cipherobj theCipherObj;
                theCipherObj.untranslated_symbol=symbol;
                theWord.push_back(theCipherObj);

            }
            theSentence.push_back(theWord);
        }
        TheCipher.push_back(theSentence);
    }
}

void cipher::UpdateSolution()
{
    for(const auto& line : TheCipher)
    {
        for(const auto& word : line)
        {
            for(const auto& symbolObj : word)
            {
                const auto& symbol = symbolObj.untranslated_symbol;
                if(not TheSolution.contains(symbol))
                {
                    TheSolution.insert(symbol, QString(""));
                }
            }
        }
    }
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
            if(std::any_of(word.constBegin(), word.constEnd(), [&](cipherobj obj) {return obj.untranslated_symbol == symbol;}))
            {
                found=true;
                break;
            }
        }
    }
    return found;
}

void cipher::on_customContextMenuRequested(QPoint pos)
{
    QPoint globalPos = ui->textEdit->mapToGlobal(pos);

    TheMenu.move(globalPos);
    TheMenu.show();

    QTextCursor textEditCursor(ui->textEdit->textCursor());

    textEditCursor.select(QTextCursor::BlockUnderCursor);
    CurrentSymbol = textEditCursor.selection().toPlainText();

    textEditCursor.select(QTextCursor::WordUnderCursor);
    CurrentWord = textEditCursor.selection().toPlainText();

    qDebug() << "Selected symbol=" << CurrentSymbol << " Selected word=" << CurrentWord;

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
    ui->verticalLayout->removeWidget(ui->solver);
    delete ui->solver;
    QWidget *solver = new QWidget(ui->solverframe);
    solver->setObjectName(QString::fromUtf8("Solver"));
    solver->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ui->solver = solver;

    QVBoxLayout *lineLayout = new QVBoxLayout(solver);
    lineLayout->setObjectName(QString::fromUtf8("SolverVerticalLayout"));
    lineLayout->addStretch(1);

    int lineCount = 0;
    for(const auto& line : TheCipher)
    {
        QGroupBox *wordsGroupBox = new QGroupBox(solver);

        wordsGroupBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        QHBoxLayout *wordsLayout = new QHBoxLayout(wordsGroupBox);
        wordsLayout->setObjectName(QString::fromUtf8("SolverHorizontalLayout"));
        wordsLayout->addStretch(1);
        wordsGroupBox->setLayout(wordsLayout);
        wordsLayout->addSpacerItem(new QSpacerItem(10,0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

        int wordCount = 0;
        for(const auto& word: line)
        {
            QLineEdit *wordEditor = new QLineEdit(lineLayout->widget());
            wordEditor->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            wordEditor->setFont(QFont("Courier, 12"));
            wordEditor->setObjectName(QString("solver_%1_%2").arg(lineCount).arg(wordCount));
            QString translation;
            for(const auto& cipherObj: word)
            {
                translation.append(cipherObj.translated_symbol);
            }
            wordEditor->setText(translation);
            connect(wordEditor, &QLineEdit::textChanged, this, &cipher::solver_textChanged);
            wordsLayout->addWidget(wordEditor);
            wordsLayout->addSpacerItem(new QSpacerItem(10,0, QSizePolicy::Expanding, QSizePolicy::Expanding));
            ++wordCount;
        }
        lineLayout->addSpacerItem(new QSpacerItem(0, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));
        ++lineCount;
    }
    ui->verticalLayout->insertWidget(2,solver);
    solver->show();
}

void cipher::solver_textChanged(const QString& text)
{
    QString lineEditor(sender()->objectName());
    QRegularExpression re("^Solver_(?<Line>%d+)_(?<Word>%d+)");
    QRegularExpressionMatch cipherMatcher = re.match(text);
    int line(cipherMatcher.captured("Line").toInt());
    int word(cipherMatcher.captured("Word").toInt());

    int symbol_index = 0;
    for(const auto& symbol : text.split("", QString::SkipEmptyParts))
    {
        cipherobj& unknownSymbol(TheCipher[line][word][symbol_index]);
        bool translated(symbol=="_");
        unknownSymbol.translated = translated;
        if(translated)
        {
            unknownSymbol.translated_symbol = symbol;
        }

        ++symbol_index;
    }
}
