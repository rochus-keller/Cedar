#ifndef _CEDAR_LEXER
#define _CEDAR_LEXER

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

#include <CedarToken.h>
#include <QList>
#include <QIODevice>

namespace Cedar
{

class Lexer
{
public:
    Lexer();
    ~Lexer();

    void setStream(QString code, const QString& filePath = QString());
    void setIgnoreComments( bool b ) { d_ignoreComments = b; }
    void setPackComments( bool b ) { d_packComments = b; }

    Token nextToken();
    Token peekToken(quint8 lookAhead = 1);
    QList<Token> tokens( QString code );
    quint32 getSloc() const { return d_sloc; }

    static const char negSym;
protected:
    Token nextTokenImp();
    int skipWhiteSpace();
    void nextLine();
    int lookAhead(int off = 1) const;
    Token token(TokenType tt, int len = 1, const QByteArray &val = QByteArray());
    Token ident();
    Token number();
    Token symbol();
    Token comment();
    Token character();
    Token string();
    void countLine();
private:
    QIODevice* d_in;
    quint32 d_lineNr;
    quint16 d_colNr;
    QByteArray d_line;
    QList<Token> d_buffer;
    Token d_lastToken;
    quint32 d_sloc; // number of lines of code without empty or comment lines
    QString d_filePath;
    bool d_ignoreComments;  // don't deliver comment tokens
    bool d_packComments;    // Only deliver one Tok_Comment for /**/ instead of Tok_Lcmt and Tok_Rcmt
    bool d_lineCounted;
};

}

#endif
