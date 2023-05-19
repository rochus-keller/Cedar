#ifndef TIOGAVIEWER_H
#define TIOGAVIEWER_H

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

#include <QMainWindow>

class QTreeWidget;
class QTreeWidgetItem;
class QTextBrowser;
class QPlainTextEdit;
class QStackedWidget;
class QLabel;

class TiogaViewer : public QMainWindow
{
    Q_OBJECT
public:
    explicit TiogaViewer(QWidget *parent = 0);
    void setRootPath( const QString& );
    void openFile( const QString& );
    void parseFile(const QString& code, const QString& );
signals:

protected slots:
    void onFileClicked(QTreeWidgetItem*,int);
    void onErrsClicked(QTreeWidgetItem*,int);
    void onOpen();
protected:
    void createFileTree();
    void createErrs();
private:
    QTreeWidget* d_fileTree;
    QTextBrowser* d_docViewer;
    QPlainTextEdit* d_codeViewer;
    QString d_root;
    QLabel* d_title;
    QStackedWidget* d_switch;
    QTreeWidget* d_errs;
};

#endif // TIOGAVIEWER_H
