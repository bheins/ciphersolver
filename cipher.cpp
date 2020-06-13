﻿#include "cipher.h"
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
#include <QFileDialog>
#include <QVariant>
#include <QDateTime>
#include <QPlainTextEdit>
#include <limits>
#include <QFileInfo>
#include <QAction>
#include <QMessageBox>
#include <QStyle>

cipher::cipher(QWidget *parent)
    : QMainWindow(parent)
    , CurrentSymbol("")
    , CurrentWord("")
    , TheCipher()
    , TheSolution()
    , ui(new Ui::CipherUI)
    , SettingsFile("cipher.ini")
    , Settings(SettingsFile, QSettings::IniFormat)
    , CipherChangeNotSaved(false)
    , SolverChangeNotSaved(false)
{
//    qRegisterMetaType<Cipher>("Cipher");
    ui->setupUi(this);
    load_dictionary();
    update_recent_files();
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

void cipher::on_Cipher_textChanged()
{
    CipherChangeNotSaved = true;
    update_cipher();
//    remove_unused_symbols();
    build_interactive_solver();
}

void cipher::remove_unused_symbols()
{
}

void cipher::update_cipher()
{
    QString newText(ui->Cipher->toPlainText());
    TheCipher.clear();
    TheCipher.reserve(newText.size());//includes spaces, but will be large enough

    int index = 0;
    for(const auto& sentence : newText.split("\n", Qt::SkipEmptyParts))
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
                if(nullptr == find_cipher_by_untranslated_symbol(symbol))
                {
                    TheSolution[index]=new cipherobj(symbol);
                }
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

void cipher::show_custom_word_menu_selection(QPoint pos)
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
    QVBoxLayout* solverVerticalLayout = ui->VerticalLayout;
    QWidget* solver = ui->SolverMain->findChild<QWidget*>("Solver");

    QWidget* cipher = nullptr;
    if(nullptr not_eq solver)
    {
        QLayoutItem *wItem;
        while((wItem = solverVerticalLayout->takeAt(0)) != 0)
        {
            QWidget* item=wItem->widget();
            if(nullptr not_eq item)
            {
                if(item->objectName() == "Cipher")
                {
                    cipher=item;
                }
                else if(nullptr not_eq item)
                {
                    item->setParent(nullptr);
                    delete wItem;
                }
            }
        }
        solverVerticalLayout->removeWidget(solver);
        solver->setParent(nullptr);
        delete solver;
    }

    solverVerticalLayout->addWidget(cipher);
    solver = new QWidget(ui->SolverMain);
    solver->setObjectName("Solver");
    solver->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    int lineCount = 0;
    for(const auto& line : TheCipher)
    {
        QWidget *lineGroupBox = new QWidget(solver);
        lineGroupBox->setObjectName(QString("LineGroup%1")
                                    .arg(QString::number(lineCount)));
        lineGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        QHBoxLayout *lineLayout = new QHBoxLayout(lineGroupBox);
        lineLayout->setMargin(5);
        lineLayout->setSpacing(5);
        lineLayout->setSizeConstraint(QLayout::SetFixedSize);

        int wordCount = 0;
        for(const auto& word: line)
        {
            QWidget *wordGroupBox = new QWidget(lineGroupBox);
            wordGroupBox->setObjectName(QString("WordGroup%1_%2")
                                        .arg(QString::number(lineCount))
                                        .arg(QString::number(wordCount)));
            wordGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            QVBoxLayout *wordsLayout = new QVBoxLayout(wordGroupBox);
            wordsLayout->setSizeConstraint(QLayout::SetFixedSize);
            wordsLayout->setMargin(0);
            wordsLayout->setSpacing(5);

            QMap<int, cipherobj*> wordTranslation;
            int symbolCount=0;
            for(const auto& symbol: word)
            {
                wordTranslation.insert(symbolCount, find_cipher_by_untranslated_symbol(symbol));
                ++symbolCount;
            }
            QString lineEditTranslation;
            for(const auto& cobj: wordTranslation)
            {
                lineEditTranslation.append(cobj->get_untranslated_symbol());
            }
            CipherWordLineEdit *wordEditor = new CipherWordLineEdit(wordTranslation, lineCount, wordCount, wordGroupBox);
            wordEditor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            wordEditor->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(wordEditor, &QLineEdit::textChanged, this, &cipher::solver_textChanged, Qt::UniqueConnection);
            connect(wordEditor, &QLineEdit::customContextMenuRequested, this, &cipher::show_custom_word_menu_selection);

            QLineEdit* untranslatedWordText = new QLineEdit(wordGroupBox);
            untranslatedWordText->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            untranslatedWordText->setReadOnly(true);
            untranslatedWordText->setStyleSheet("* { background-color: rgba(0, 0, 0, 0); }");
            untranslatedWordText->setFrame(false);

            QString untranslated;
            for(const auto& symbol: word)
            {
                untranslated.append(symbol);
            }
            untranslatedWordText->setText(untranslated);
            untranslatedWordText->setObjectName(QString("untranslated%1_%2")
                                                .arg(QString::number(lineCount))
                                                .arg(QString::number(wordCount)));
            QFont wordFont("Courier", 16, QFont::Normal);
            untranslatedWordText->setFont(wordFont);
            QFontMetrics fm(wordFont);
            untranslatedWordText->setFixedSize(fm.horizontalAdvance(untranslated)+untranslatedWordText->textMargins().left()+untranslatedWordText->textMargins().right()+4,
                                 fm.height()+untranslatedWordText->textMargins().top()+untranslatedWordText->textMargins().bottom()+4);
            wordsLayout->addWidget(wordEditor);
            wordsLayout->addWidget(untranslatedWordText);
            lineLayout->addWidget(wordGroupBox);
            wordGroupBox->show();
            ++wordCount;
        }
        ui->VerticalLayout->addWidget(lineGroupBox);
        lineGroupBox->show();
        ++lineCount;
    }
}

void cipher::solver_textChanged(const QString& text)
{
    SolverChangeNotSaved = true;
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
                untranslated_cipher_obj->add_translation(symbol);
            }
            else if(symbol not_eq "-")
            {
                if(untranslated_cipher_obj->get_translated_symbol() not_eq symbol)
                {
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

void cipher::actionOther_triggered(bool)
{
    QString fileName = Settings.value("LastSaveFile").toString();
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Load Cipher"), fileName,
                                            tr("Cipher (*.cpr);;All Files (*)"));
    open_cipher(fileName);
}

bool cipher::save_cipher(const QString& filename)
{
    bool save_success=false;

    QSettings saveFile(filename, QSettings::IniFormat);
    saveFile.setValue("Cipher", QVariant::fromValue(ui->Cipher->toPlainText()));
    saveFile.beginGroup("Solution");

    for(const auto& cobj : TheSolution)
    {
        if(cobj->get_translated_symbol() not_eq "-")
        {
            saveFile.setValue(cobj->get_untranslated_symbol(), cobj->get_translated_symbol());
        }
    }
    saveFile.endGroup();
    saveFile.sync();
    if(QSettings::AccessError == saveFile.status())
    {
        qDebug() << "Error creating file " << filename;
    }
    else
    {
        Settings.beginGroup("Save");
        Settings.setValue("LastSaveFile", filename);
        QDateTime now=QDateTime::currentDateTimeUtc();
        Settings.setValue("LastSavedTime", now);
        Settings.beginGroup("MostRecent");

        QMap<QString, QDateTime> mostRecent;
        for(const auto& file : Settings.allKeys())
        {
            mostRecent[file]=qvariant_cast<QDateTime>(Settings.value(file));
        }
        if(mostRecent.size()==5)
        {
            QDateTime oldestDate(*std::min_element(mostRecent.constBegin(), mostRecent.constEnd()));
            qDebug() << "Oldest file=" << mostRecent.key(oldestDate);
            Settings.remove(mostRecent.key(oldestDate));
        }
        Settings.setValue(filename, now);
        Settings.endGroup();
        Settings.endGroup();
        Settings.sync();
        CipherChangeNotSaved = false;
        SolverChangeNotSaved = false;
        save_success=true;
    }
    return save_success;
}

void cipher::save_cipher_helper(bool allowDiscard)
{
    bool finished=false;
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Cipher"), "",
            tr("Cipher (*.cpr);;All Files (*)"));

    finished = save_cipher(fileName);
    if(not finished)
    {
        QMessageBox retry;
        retry.setText("Save failed.");
        retry.setInformativeText("Do you want to save your changes?");
        retry.setStandardButtons(QMessageBox::Save | (allowDiscard ? QMessageBox::Discard : QMessageBox::NoButton) | QMessageBox::Cancel);
        retry.setDefaultButton(QMessageBox::Save);

        switch(retry.exec())
        {
        case QMessageBox::Save:
            save_cipher_helper(allowDiscard);
            break;
        case QMessageBox::Discard:
            CipherChangeNotSaved=false;
            SolverChangeNotSaved=false;
            on_actionClose_triggered(false);
            break;
        }
    }
    else
    {
        CipherChangeNotSaved=false;
        SolverChangeNotSaved=false;
    }
}

void cipher::on_actionSave_As_triggered(bool)
{
    save_cipher_helper(false);
}

void cipher::on_actionSave_triggered(bool)
{
    QString lastSaveFile = Settings.value("Save/LastSaveFile").toString();

    if(lastSaveFile.isEmpty())
    {
        on_actionSave_As_triggered(false);
    }
    else
    {
        save_cipher(lastSaveFile);
    }
}

void cipher::reset()
{
    if(ui->Cipher->toPlainText().size() > 0)
    {
        ui->Cipher->clear();
    }
    qDeleteAll(TheSolution);
    TheSolution.clear();
}

void cipher::on_actionClose_triggered(bool)
{
    if(CipherChangeNotSaved or SolverChangeNotSaved)
    {
        QMessageBox saveChanges;
        saveChanges.setText(QString("Changes to text of the %1 have not been saved.").arg(CipherChangeNotSaved ? "Cipher" : "Solver"));
        saveChanges.setInformativeText("Do you want to save your changes?");
        saveChanges.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        saveChanges.setDefaultButton(QMessageBox::Save);

        switch(saveChanges.exec())
        {
        case QMessageBox::Save:
            save_cipher_helper(true);
            [[fallthrough]];
        case QMessageBox::Discard:
            reset();
            break;
        default:
            break;
        }
    }
    else
    {
        reset();
    }
}

void cipher::on_actionExit_triggered(bool)
{
    on_actionClose_triggered(true);
    close();
}

void cipher::open_cipher(const QString& filename)
{
    if(CipherChangeNotSaved or SolverChangeNotSaved)
    {
        QMessageBox save;
        save.setText("Save Changes?");
        save.setInformativeText("Do you want to save your changes?");
        save.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
        save.setDefaultButton(QMessageBox::Save);

        switch(save.exec())
        {
        case QMessageBox::Save:
            save_cipher_helper(true);
            break;
        default:
            CipherChangeNotSaved = false;
            SolverChangeNotSaved = false;
            on_actionClose_triggered(false);
            break;
        }
    }

    QSettings loadFile(filename, QSettings::IniFormat);
    reset();
    ui->Cipher->setPlainText(loadFile.value("Cipher").toString());
    loadFile.beginGroup("Solution");
    for(const auto& symbol : loadFile.childKeys())
    {
        find_cipher_by_untranslated_symbol(symbol)->add_translation(loadFile.value(symbol).toString());
    }
    loadFile.endGroup();

    update_recent_files();
}

void cipher::update_recent_files()
{
    for(auto action: ui->menuOpen->actions())
    {
        ui->menuOpen->removeAction(action);
    }
    Settings.beginGroup("Save");
    Settings.beginGroup("MostRecent");
    QList<QAction*> mostRecent;
    for(const auto& file : Settings.allKeys())
    {
        QAction* newAction=new QAction(QFileInfo(file).baseName());
        newAction->setData(file);
        mostRecent.append(newAction);
        connect(newAction, &QAction::triggered, this, [&](bool) { open_cipher(qobject_cast<QAction*>(sender())->data().toString());});
    }
    Settings.endGroup();
    Settings.endGroup();

    ui->menuOpen->addActions(mostRecent);
    ui->menuOpen->addSeparator();
    QAction *newAction=new QAction("Other ...");
    connect(newAction, &QAction::triggered, this, [&](bool) { actionOther_triggered(false);});
    ui->menuOpen->addAction(newAction);
}
