/*
* Copyright 2023 Rochus Keller <mailto:me@rochus-keller.ch>
*
* This file is part of the Lisa Pascal Navigator application.
*
* The following is the license that applies to this copy of the
* application. For a license to use the library under conditions
* other than those described here, please email to me@rochus-keller.ch.
*
* GNU General Public License Usage
* This file may be used under the terms of the GNU General Public
* License (GPL) versions 2.0 or 3.0 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in
* the packaging of this file. Please review the following information
* to ensure GNU General Public Licensing requirements will be met:
* http://www.fsf.org/licensing/licenses/info/GPLv2.html and
* http://www.gnu.org/copyleft/gpl.html.
*/

// * Adopted from https://github.com/rochus-keller/Oberon/

#include "CedarHighlighter.h"
#include "CedarLexer.h"
#include <QBuffer>
using namespace Cedar;

Highlighter::Highlighter(QTextDocument* parent) :
    QSyntaxHighlighter(parent)
{
    for( int i = 0; i < C_Max; i++ )
    {
        d_format[i].setFontWeight(QFont::Normal);
        d_format[i].setForeground(Qt::black);
        d_format[i].setBackground(Qt::transparent);
    }
    d_format[C_Num].setForeground(QColor(0, 153, 153));
    d_format[C_Str].setForeground(QColor(208, 16, 64));
    d_format[C_Sym].setForeground(QColor(210,105,30));
    d_format[C_Cmt].setForeground(QColor(153, 153, 136));
    d_format[C_Kw].setForeground(QColor(68, 85, 136));
    d_format[C_Kw].setFontWeight(QFont::Bold);
    d_format[C_Op].setForeground(QColor(153, 0, 0));
    d_format[C_Op].setFontWeight(QFont::Bold);
    d_format[C_Type].setForeground(QColor(153, 0, 115));
    d_format[C_Type].setFontWeight(QFont::Bold);
    d_format[C_Pp].setFontWeight(QFont::Bold);
    d_format[C_Pp].setForeground(QColor(0, 128, 0));
    d_format[C_Pp].setBackground(QColor(230, 255, 230));
    d_format[C_Label].setForeground(QColor(251, 138, 0));
    d_format[C_Label].setBackground(QColor(253, 217, 165));
}

void Highlighter::addBuiltIn(const QByteArray& bi)
{
    d_builtins << bi;
}

void Highlighter::addKeyword(const QByteArray& kw)
{
    d_keywords << kw;
}

QTextCharFormat Highlighter::formatForCategory(int c) const
{
    return d_format[c];
}

void Highlighter::highlightBlock(const QString& text)
{
    const int previousBlockState_ = previousBlockState();
    int lexerState = 0, initialBraceDepth = 0;
    if (previousBlockState_ != -1) {
        lexerState = previousBlockState_ & 0xff;
        initialBraceDepth = previousBlockState_ >> 8;
    }

    int braceDepth = initialBraceDepth;

    // Protocol:
    // lexerState == 1: multi-line << >> comment

    int start = 0;
    if( lexerState == 1 )
    {
        QTextCharFormat f = formatForCategory(C_Cmt);
        int pos = text.indexOf(">>");
        if( pos == -1 )
        {
            // the whole block ist part of comment
            setFormat( start, text.size(), f );
            setCurrentBlockState( (braceDepth << 8) | lexerState);
            return;
        }else
        {
            // End of comment found
            pos += 2;
            setFormat( start, pos , f );
            lexerState = 0;
            braceDepth--;
            start = pos;
        }
    }

    Cedar::Lexer lex;
    lex.setIgnoreComments(false);
    lex.setPackComments(false);

    QList<Token> tokens = lex.tokens(text.mid(start));
    for( int i = 0; i < tokens.size(); ++i )
    {
        Token &t = tokens[i];
        t.d_colNr += start;

        QTextCharFormat f;
        if( t.d_type == Tok_Comment )
            f = formatForCategory(C_Cmt); // one line comment
        else if( t.d_type == Tok_2Lt )
        {
            f = formatForCategory(C_Cmt);
            lexerState = 1;
            if( t.d_val.endsWith(">>") )
                lexerState = 0;
            else
                braceDepth++;
        }else if( t.d_type == Tok_string || t.d_type == Tok_char )
            f = formatForCategory(C_Str);
        else if( t.d_type == Tok_number )
            f = formatForCategory(C_Num);
        else if( tokenTypeIsLiteral(t.d_type) )
        {
            f = formatForCategory(C_Op);
        }else if( tokenTypeIsKeyword(t.d_type) )
        {
            f = formatForCategory(C_Kw);
        }else if( t.d_type == Tok_n )
        {
            /*if( i+1 < tokens.size() && tokens[i+1].d_type == Tok_Colon)
                f = formatForCategory(C_Label);
            else */if( d_builtins.contains(t.d_val) )
                f = formatForCategory(C_Type);
            else if( d_keywords.contains(t.d_val) )
                f = formatForCategory(C_Kw);
            else
                f = formatForCategory(C_Ident);
        }else if( t.d_type == Tok_symbol )
            f = formatForCategory(C_Sym);

        /*if( lexerState == 3 )
            setFormat( startPp, t.d_colNr - startPp + t.d_val.size(), formatForCategory(C_Pp) );
        else */
        if( f.isValid() )
            setFormat( t.d_colNr-1, t.d_val.isEmpty() ? t.d_len : t.d_val.size(), f );
    }

    setCurrentBlockState((braceDepth << 8) | lexerState );
}



LogPainter::LogPainter(QTextDocument* parent):QSyntaxHighlighter(parent)
{

}

void LogPainter::highlightBlock(const QString& text)
{
    QColor c = Qt::black;
    if( text.startsWith("WRN:") )
        c = Qt::blue;
    else if( text.startsWith("ERR:") )
        c = Qt::red;

    setFormat( 0, text.size(), c );
}



