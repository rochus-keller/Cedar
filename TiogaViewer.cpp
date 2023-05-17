/*
** Copyright (C) 2023 Rochus Keller (me@rochus-keller.ch)
**
** This file is part of the Cedar/Mesa project.
**
** $QT_BEGIN_LICENSE:LGPL21$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

#include "TiogaReader.h"
#include "TiogaViewer.h"
#include "CedarHighlighter.h"
#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QLabel>
#include <QPlainTextEdit>
#include <QShortcut>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QTreeWidget>
#include <QVBoxLayout>

TiogaViewer::TiogaViewer(QWidget *parent) : QMainWindow(parent)
{
    QWidget* pane = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(pane);
    vbox->setMargin(0);

    d_title = new QLabel(this);
    d_title->setWordWrap(true);
    d_title->setTextInteractionFlags(Qt::TextSelectableByMouse);
    vbox->addWidget(d_title);

    d_switch = new QStackedWidget(this);
    vbox->addWidget(d_switch);

    d_docViewer = new QTextBrowser(this);
    d_switch->addWidget(d_docViewer);

    d_codeViewer = new QPlainTextEdit(this);
    d_codeViewer->setReadOnly(true);
    d_codeViewer->setLineWrapMode( QPlainTextEdit::NoWrap );
    d_codeViewer->setTabStopWidth( 30 );
    d_codeViewer->setTabChangesFocus(true);
    d_codeViewer->setMouseTracking(true);
    Cedar::Highlighter* hl = new Cedar::Highlighter(d_codeViewer->document());
    const QByteArrayList builtins
            = QByteArrayList() << "ATOM" << "BOOL" << "BOOLEAN" << "CARDINAL" << "CHAR" << "CHARACTER" << "CODE"
                               << "ELSE" << "ISTYPE" << "PACKED" << "SIGNAL" << "ENABLE" << "JOIN" << "PAINTED"
                               << "SIZE" << "END" << "LAST" << "POINTER" << "START" << "ENDCASE" << "LENGTH" << "PORT"
                               << "STATE" << "ENDLOOP" << "LIST" << "PRED" << "STOP" << "ENTRY" << "LOCKS" << "PRIVATE"
                               << "STRING" << "ERROR" << "LONG" << "PROC" << "SUCC" << "EXIT" << "LOOP" << "PROCEDURE"
                               << "TEXT" << "EXITS" << "LOOPHOLE" << "PROCESS" << "THEN" << "EXPORTS" << "MACHINE"
                               << "PROGRAM" << "THROUGH" << "FINISHED" << "MAX" << "PUBLIC" << "TO" << "FIRST" << "MIN"
                               << "READONLY" << "TRANSFER" << "FOR" << "MOD" << "RECORD" << "TRASH" << "FORK"
                               << "MONITOR" << "REF" << "TRUSTED" << "FRAME" << "MONITORED" << "REJECT" << "TYPE"
                               << "FREE" << "NARROW" << "RELATIVE" << "UNCHECKED" << "FROM" << "NEW" << "REPEAT"
                               << "UNCOUNTED" << "GO" << "NIL" << "RESTART" << "UNTIL" << "GOTO" << "NOT" << "RESUME"
                               << "USING" << "IF" << "NOTIFY" << "RETRY" << "WAIT" << "IMPORTS" << "NULL" << "RETURN"
                               << "WHILE" << "IN" << "OF" << "RETURNS" << "WITH" << "INLINE" << "OPEN" << "SAFE"
                               << "ZONE" << "INT" << "OR" << "SELECT" << "INTEGER" << "ORDERED" << "SEQUENCE"
                               << "INTERNAL" << "OVERLAID" << "SHARES"
                               << "TRUE" << "FALSE" << "CARD";
    foreach( const QByteArray& bi, builtins )
        hl->addBuiltIn(bi);

#if defined(Q_OS_WIN32)
    QFont monospace("Consolas");
#elif defined(Q_OS_MAC)
    QFont monospace("SF Mono");
#else
    QFont monospace("Monospace");
#endif
    if( !monospace.exactMatch() )
        monospace = QFont("DejaVu Sans Mono");
    d_codeViewer->setFont(monospace);
    d_switch->addWidget(d_codeViewer);

    setCentralWidget(pane);
    createFileTree();

    new QShortcut(tr("CTRL+O"),this,SLOT(onOpen()));
    new QShortcut(tr("CTRL+Q"),this,SLOT(close()));
}

template<class T>
static bool fillFiles( T* parent, const QDir& dir, const QStringList& suffix )
{
    const QStringList dirs = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );

    bool hasFiles = false;
    foreach( const QString& d, dirs )
    {
        const QString path = dir.absoluteFilePath(d);
        QTreeWidgetItem* item = new QTreeWidgetItem(parent,QFileIconProvider::Folder);
        item->setText(0,d);
        item->setToolTip(0,path);
        QFileIconProvider icon;
        item->setIcon(0,icon.icon(QFileIconProvider::Folder));
        if( !fillFiles( item, path, suffix ) )
            delete item;
        else
            hasFiles = true;
    }

    const QStringList files = dir.entryList( suffix, QDir::Files, QDir::Name );
    foreach( const QString& f, files )
    {
        const QString path = dir.absoluteFilePath(f);
        QTreeWidgetItem* item = new QTreeWidgetItem(parent,QFileIconProvider::File);
        item->setText(0,f);
        item->setToolTip(0,path);
        QFileIconProvider icon;
        item->setIcon(0,icon.icon(QFileIconProvider::File));
        hasFiles = true;
    }
    return hasFiles;
}

static QStringList filter()
{
    static const char* suffix[] = {
        "tioga",
        "mesa",
        "df",
        "require",
        "profile",
        "depends",
        0
    };
    QStringList res;
    const char** p = suffix;
    while( *p )
    {
        res << QString("*.%1").arg(*p) << QString("*.%1!*").arg(*p);
        p++;
    }
    return res;
}

void TiogaViewer::setRootPath(const QString& path)
{
    d_root = path;
    d_fileTree->clear();
    d_docViewer->clear();
    d_codeViewer->clear();
    d_title->clear();
    setWindowTitle(tr("%1 - TiogaViewer").arg(path));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    fillFiles( d_fileTree, path, filter() );
    QApplication::restoreOverrideCursor();
}

void TiogaViewer::openFile(const QString& file)
{
    const QString rfile = file.mid(d_root.size());
    d_title->setText(rfile);
    QFile in(file);
    if( in.open(QIODevice::ReadOnly) )
    {
        const bool isCode = file.endsWith(".mesa") || file.contains(".mesa!");
        TiogaReader r;
        if( !r.read( in.readAll(), rfile, isCode ) )
            d_title->setText(QString("error reading file: %1").arg(rfile));
        else
        {
            if( isCode )
            {
                d_switch->setCurrentWidget(d_codeViewer);
                d_codeViewer->setPlainText(r.text);
            }else
            {
                d_switch->setCurrentWidget(d_docViewer);
                d_docViewer->setText(r.text);
            }
        }
    }else
        d_title->setText(QString("cannot open file for reading: %1").arg(rfile));
}

void TiogaViewer::onFileClicked(QTreeWidgetItem* item,int)
{
    d_docViewer->clear();
    d_codeViewer->clear();
    if( item->type() == QFileIconProvider::File )
    {
        const QString file = item->toolTip(0);
        openFile(file);
    }
}

void TiogaViewer::onOpen()
{
    const QString path = QFileDialog::getExistingDirectory(this,
                           "Select Root Directory of Source Tree", d_root );
    if( path.isEmpty() )
        return;
    setRootPath(path);
}

void TiogaViewer::createFileTree()
{
    QDockWidget* dock = new QDockWidget( tr("Files"), this );
    dock->setObjectName("Files");
    dock->setAllowedAreas( Qt::AllDockWidgetAreas );
    dock->setFeatures( QDockWidget::DockWidgetMovable );
    d_fileTree = new QTreeWidget(dock);
    d_fileTree->setAlternatingRowColors(true);
    d_fileTree->setSortingEnabled(false);
    d_fileTree->setAllColumnsShowFocus(true);
    d_fileTree->setRootIsDecorated(true);
    d_fileTree->setHeaderHidden(true);
    dock->setWidget(d_fileTree);
    addDockWidget( Qt::LeftDockWidgetArea, dock );
    connect( d_fileTree,SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onFileClicked(QTreeWidgetItem*,int)) );
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TiogaViewer w;
    if( a.arguments().size() >= 2 )
    {
        QFileInfo info(a.arguments()[1]);
        if( info.isFile() )
            w.openFile(info.absoluteFilePath());
        else
            w.setRootPath(info.absoluteFilePath());
    }
    w.showMaximized();

    return a.exec();
}
