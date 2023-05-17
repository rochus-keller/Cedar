/*
** Copyright (c) 2023 Rochus Keller (me@rochus-keller.ch)
** Copyright (c) 1993 Xerox Corporation
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
#include <QIODevice>
#include <QTextCursor>
#include <QtDebug>
#include <QTextStream>

#define tioga_NumFormats	70
#define tioga_NumLooks		50
#define tioga_NumProps		50
#define tioga_LenLen		4 /* length of a length field */
#define tioga_IDLen		2 /* id fields are all two bytes */
#define tioga_CommentHeaderLen	(tioga_IDLen + tioga_LenLen)
#define tioga_ControlHeaderLen	(tioga_IDLen + tioga_LenLen)
#define tioga_TrailerLen	(tioga_IDLen + 3 * tioga_LenLen)

enum tioga_ControlOp {
    endOfFile = 0,
    startNode,
    /** length of formatName in next byte.  Text for format name
        follows that.
        enter in formatName table
        and assign it the next number */
    startNodeFirst,
    startNodeLast = startNodeFirst + tioga_NumFormats,
    /* these opcodes encode previously seen formatName so don't need
       to repeat it find formatName in op-startNodeFirst of format
       table other information follows same as for startNode */
    terminalTextNode,
    terminalTextNodeFirst,
    terminalTextNodeLast = terminalTextNodeFirst + tioga_NumFormats,
    /* these opcodes are for nodes without children so can skip
       endNode opcode identical to startNodeFirst..startNodeLast,
       except implies no children find format name in
       op-startNodeFirst of format table other information follows
       same as for startNode */
    otherNode,
    /** for "other" format of nodes length of formatName in next byte.
        text for format name follows that.
        enter in formatName table
        and assign it the next number */
    otherNodeShort,
    /* like otherNode, but followed by a formatName code number
       instead of length+text */
    otherNodeSpecs,
    /** Gives "variety" and specifications for immediately previous
       "other" format node.  Length of variety name follows in next
       byte(s).  Then text of name.
        Enter in property name table and assign next number.
       Length of specs text in next byte(s) specs follow that. */
    otherNodeSpecsShort,
    /* like otherNodeSpecs, but followed by a propname code number
       instead of length+text */
    prop,
    /* Specifies property for current node. Property specs are stored
       on file as a rope. Length of property name follows in next
       byte(s). then text of name. Enter in property name table and
       assign next number. Length of property specs rope in next
       byte(s). Followed by text for property specs rope. */
    propShort,
    /* Like prop, but followed by a propname code number instead of
       length+text. */
    endNode,
    /* End current node and go back to adding to its parent. */
    rope,
    /* This op declares rope for most recently started node. Length of
       text for the node in next byte(s). Actual text comes from text
       block followed by a CR which is not included in the length. */
    comment,
    /* Identical to rope except implies text stored in comment area of
       file. */
    runs,
    /* This op preceeds definition of looks for most recently started
       node. Number of runs in following byte(s). Have at most 1 runs
       op per node.  if omit, then no special looks. */
    looks,
    /** Looks vector in following 4 bytes.
        Enter vector in looks table.
        and assign it the next number
        Length of run in next byte(s). */
    looksFirst,
    looksLast = looksFirst + tioga_NumLooks,
    /* These ops encode previously encountered looks so don't need to
       repeat. Find looks in looks table[op-looksFirst]. Length of run
       in the next byte(s). */
    look1,
    /* Like looks op, except has single look char instead of 4 byte
       looks vector. */
    look2,
    /* Like looks1, except has two looks chars. */
    look3,
    /* Like look2, except has three looks chars. */
};

struct tread_Stream {
    unsigned char *next;
    unsigned char *limit;
};

struct tread_Reader {
    QTextStream out;
    bool isCode;

    /* Full buffer. */
    const char *buf;
    int totalLen;
    /* Start and end of each region. */
    struct tread_Stream text;
    struct tread_Stream com;
    struct tread_Stream control;
    /* Generic piece of text */
    int strLen;
    QByteArray str;
    /* Formats */
    int nFormats;		/* number in use */
    QByteArray formats[tioga_NumFormats]; /* list of them */
    /* Look combos */
    int nLooks;			/* number in use */
    long _looks[tioga_NumLooks];	/* list of them */
    /* Property names */
    int nProps;
    QByteArray props[tioga_NumProps];

    static bool CheckID(unsigned char **pp, unsigned char *id)
    {
        unsigned char *s = *pp;
        bool ok = true;
        int i;

        for (i = 0; i < tioga_IDLen; ++i) {
            if (*s++ != *id++)
                ok = false;
        }
        *pp = s;
        return ok;
    }

    static int GetLength(unsigned char **pp)
    {
        unsigned char *s = *pp;
        long result = 0;

        result |= *s++ << 8;
        result |= *s++;
        result |= *s++ << 24;
        result |= *s++ << 16;
        *pp = s;
        return result;
    }

    bool init(const char *buf, int len, bool code)
    {
        tread_Reader* r = this;
        unsigned char *p;
        unsigned char *ubuf = (unsigned char *)buf;
        int propLen, textLen, commentLen, totalControlLen, totalSize;
        static unsigned char trailerID[] = { 0x85, 0x97 };
        static unsigned char commentID[] = { 0, 0 };
        static unsigned char controlID[] = { 0x9d, 0xca };

        r->isCode = code;
        r->buf = buf;
        r->totalLen = len;
        /* Find the three main parts and verify that the lengths are ok. */
        p = ubuf + len - tioga_TrailerLen;
        if ( !CheckID(&p, trailerID) )
            return false;
        propLen = GetLength(&p);
        textLen = GetLength(&p);
        totalSize = GetLength(&p);
        if (totalSize != len || propLen > len || textLen > len)
            return false;
        p = ubuf + textLen;
        if ( !CheckID(&p, commentID) )
            return false;
        commentLen = GetLength(&p);
        p = ubuf + textLen + commentLen;
        if ( !CheckID(&p, controlID) )
            return false;
        totalControlLen = GetLength(&p);
        if (len != textLen + commentLen + totalControlLen)
            return false;

        /* Find start and end of each section of the file. */
        r->text.next = ubuf;
        r->text.limit = r->text.next + textLen;
        r->com.next = ubuf + textLen + tioga_CommentHeaderLen;
        r->com.limit = ubuf + textLen + commentLen;
        r->control.next = ubuf + textLen + commentLen + tioga_ControlHeaderLen;
        r->control.limit = ubuf + len - tioga_TrailerLen;

        /* Other reader state initialization. */
        r->strLen = 0;
        r->str = 0;
        /* First format is the null format. */
        r->nFormats = 1;
        r->formats[0] = NULL;
        /* First look is the empty look. */
        r->nLooks = 1;
        r->_looks[0] = 0;
        /* First property is NIL. */
        r->nProps = 1;
        r->props[0] = NULL;
        /* Preload system atoms. */
        AddProp("prefix");
        AddProp("postfix");
        return true;
    }

    int AddProp(const char *propName)
    {
        int i;

        /* Index zero reserved for NIL. */
        if (propName == NULL || propName[0] == 0)
            return 0;
        for (i = 1; i < nProps; ++i) {
            if (strcmp(props[i], propName) == 0)
                return i;
        }
        if (nProps < tioga_NumProps) {
            props[nProps] = propName;
            props[nProps] = props[nProps].toLower();
            ++nProps;
            return nProps - 1;
        }else {
            qCritical() << "Too many properties..";
            return 0;
        }
    }

    void DoWork()
    {
        tread_Reader *r = this;
        tioga_ControlOp op;
        int iFormat, iProp;
        bool terminalNode = false;
        bool lastWasTerminal = false;
        int level = 0;
        int i;
        int nRuns;
        long length;
        long runLen = 0;
        long len;

        /* main loop */
        op = GetOp();
        for (;;) {
            if (terminalTextNodeFirst <= op && op <= terminalTextNodeLast) {
                /* Get the format name, then drop down */
                iFormat = (int) op - (int) terminalTextNodeFirst;
                if (iFormat >= r->nFormats) {
                    qCritical() << "Illegal format index" << iFormat;
                    iFormat = 0;
                }
                terminalNode = true;
            }
            else if (startNodeFirst <= op && op <= startNodeLast) {
                /* Get the format name, then drop down */
                iFormat = (int) op - (int) startNodeFirst;
                if (iFormat >= r->nFormats) {
                    qCritical() << "Illegal format index" << iFormat;
                    iFormat = 0;
                }
                terminalNode = false;
            }
            else {
                switch (op) {
                case endNode:
                    /* todo */
                    if (level >= 0)
                        --level;
                    else
                        qCritical() << "Too many endNodes.";
                    EndNode();
                    op = GetOp();
                    continue;
                case startNode:
                    GetStr();
                    iFormat = AddFormat(r->str.constData());
                    terminalNode = false;
                    break;
                case terminalTextNode:
                    GetStr();
                    iFormat = AddFormat(r->str.constData());
                    terminalNode = true;
                    break;
                case rope:
                case comment:
                    length = GetInt();
                    /* Get newline, just don't pass it to client. */
                    SGetRope(op == rope ? &r->text : &r->com, length + 1);
                    /* convert newlines if should */
                    FixNewlines(r->str.data(), length + 1);
                    InsertText(r->str.constData(), length, op == comment);
                    if (runLen != 0 && runLen != length)
                        qCritical() << "Rope length(" << length << ") doesn't match run length(" << runLen << ")";
                    runLen = 0;
                    /* bump by one */
                    op = GetOp();
                    continue;
                case runs:
                    nRuns = GetInt();

                    runLen = 0;
                    for (i = 0; i < nRuns; ++i) {
                        int rl;
                        int iLook;
                        op = GetOp();
                        if (looksFirst <= op && op <= looksLast)
                            /* Look it up. */
                            iLook = (int) op - (int) looksFirst;
                        else if (look1 <= op && op <= look3)
                            iLook = GetLookChars((int) op - (int) look1 + 1);
                        else if (op == looks) {
                            /* 4 bytes of bit vector */
                            long l = GetByte();
                            l = (l << 8) | GetByte();
                            l = (l << 8) | GetByte();
                            l = (l << 8) | GetByte();
                            iLook = AddLooks(l);
                        }
                        if (iLook >= r->nLooks) {
                            qCritical() << "Look index(" << iLook << ") too large.";
                            iLook = 0;
                        }
                        rl = GetInt();
                        AddLooks(r->_looks[iLook], runLen, rl);
                        runLen += rl;
                    }
                    op = GetOp();
                    continue;
                case prop:
                    GetStr();
                    iProp = AddProp(r->str.constData());
                    len = GetInt();
                    SGetRope( &r->control, len);
                    HandleProp(r->props[iProp].constData(), r->str.constData(), len);
                    op = GetOp();
                    continue;
                case propShort:
                    iProp = GetByte();
                    if (iProp >= r->nProps) {
                        qCritical() << "Property index(" << iProp << ") out of range.";
                        iProp = 0;
                    }
                    len = GetInt();
                    SGetRope(&r->control, len);
                    HandleProp(r->props[iProp].constData(), r->str.constData(), len);
                    op = GetOp();
                    continue;
                case endOfFile:
                    if (level == 0)
                        break;	/* top level */
                    /* Supply missing endNode ops. */
                    op = endNode;
                    continue;
                case otherNode:
                case otherNodeShort:
                case otherNodeSpecs:
                case otherNodeSpecsShort:
                default:
                    qCritical() << "Illegal op code:" << op;
                    op = GetOp();
                    continue;
                }
            }
            if (level == 0 && op == endOfFile)
                break;
            /* If we make it here, then we want to start a new text node. */
            if (lastWasTerminal)
                EndNode();
            lastWasTerminal = terminalNode;
            StartNode(r->formats[iFormat]);
            if (!terminalNode)
                ++level;
            /* else stayed at same level */
            op = GetOp();
        }
    }

    tioga_ControlOp GetOp()
    {
        if (control.next < control.limit)
            return (enum tioga_ControlOp) *control.next++;
        else
            return endOfFile;
    }

    void GetStr()
    {
        long len = *control.next++;

        SGetRope(&control, len);
    }

    bool SGetRope(tread_Stream *s, long len)
    {
        long i;

        if (EnsureStrLen(len + 1) < 0)
            return false;
        for (i = 0; i < len; ++i) {
            if (s->next >= s->limit) {
                qCritical() << "Rope too long.";
                return false;
            }
            str.data()[i] = *s->next++;
        }
        str.data()[len] = 0;
        return true;
    }

    bool EnsureStrLen(long len)
    {
        if (strLen == 0) {
            strLen = len;
            str.resize(len);
        }
        else if (len > strLen) {
            strLen = len;
            str.resize(len);
        }
        return true;
    }

    static void FixNewlines(char *s, long len)
    {
        while (len > 0) {
            if (*s == '\r')
                *s = '\n';
            ++s;
            --len;
        }
    }

    long GetLookChars(int n)
    {
        long l = 0;
        int c;

        /* todo: find out about bit ordering in Mesa */
        while (n > 0) {
            c = GetByte();
            if ('a' <= c && c <= 'a' + 31)
                l |= 1 << (31 - (c - 'a'));
            else
                qCritical() << "Illegal look char:" << c;
            --n;
        }
        return AddLooks(l);
    }

    int GetByte()
    {
        if (control.next < control.limit)
            return *control.next++;
        return 0;
    }

    long GetInt()
    {
        long result = 0;
        int nBits = 0;

        while (*control.next & 0x80) {
            result |= (*control.next++ & 0x7f) << nBits;
            nBits += 7;
        }
        result |= (*control.next++ & 0x7f) << nBits;
        return result;
    }

    int AddFormat(const char *format)
    {
        int i;

        /* Index zero reserved for null format. */
        if (format == NULL || format[0] == 0)
            return 0;
        for (i = 1; i < nFormats; ++i) {
            if (strcmp(formats[i], format) == 0)
                return i;
        }
        if (nFormats < tioga_NumFormats) {
            formats[nFormats] = format;
            ++nFormats;
            return nFormats - 1;
        }
        else {
            qCritical() << "Too many formats.";
            return 0;
        }
    }

    int AddLooks(long l)
    {
        int i;

        /* Index zero reserved for empty looks. */
        if (l == 0)
            return 0;
        for (i = 1; i < nLooks; ++i) {
            if (_looks[i] == l)
                return i;
        }
        if (nLooks < tioga_NumLooks) {
            _looks[nLooks] = l;
            ++nLooks;
            return nLooks - 1;
        }
        else {
            qCritical() << "Too many looks.";
            return 0;
        }
    }

    QByteArrayList level;
    // level corresponds to indent, not primarily to titel level

    void StartNode(const char *format)
    {
        const QByteArray f = format;
        TiogaReader::__formats[f]++;
        level.push_back(format);
    }

    void EndNode()
    {
        level.pop_back();
    }

    void AddLooks(quint32 looks, long start, int len)
    {
        const QByteArray l = QByteArray::number(looks,16);
        TiogaReader::__looks[l]++;
        // qDebug() << "*** AddLooks" << l << start << len;
    }

    static QString toString( const char* in, int len )
    {
        const QString chars = QString::fromUtf8("©←");
        QString out;
        out.resize(len);
        for( int i = 0; i < len; i++ )
        {
            const char ch = in[i];
            switch( (quint8)ch )
            {
            case 0xd3: // this is 'Ó' in Latin-1 charset, convert to '©'
                out[i] = chars[0];
                break;
            case 0xac: // this is '¬' in Latin-1 charset, convert to '←'
                out[i] = chars[1];
                break;
            default:
                out[i] = QChar::fromLatin1(ch);
                break;
            }
        }
        return out;
    }

    void InsertText(const char *t, int len, bool comment)
    {
        if( isCode )
        {
            const QStringList lines = toString(t,len).split('\n');
            out << QByteArray((level.size()-2) * 4,' ');

            foreach( const QString& line, lines )
            {
                if( comment )
                {
                    const QString trimmed = line.trimmed();
                    if( !trimmed.isEmpty() && !trimmed.startsWith("--") )
                        out << "-- ";
                    if( level.size() == 1 && trimmed.isEmpty() )
                        return;
                }
                out << line << endl;
            }
        }else
        {
            QString text = toString(t,len).toHtmlEscaped();
            const QByteArray f = level.back();
            if( f.startsWith("code") )
                out << "<pre><code>" << text << "</code></pre>" << endl;
            else if( f == "head" )
            {
                QByteArray tag;
                switch (level.size()-1)
                {
                case 1:
                    tag = "h1";
                    break;
                case 2:
                    tag = "h2";
                    break;
                case 3:
                    tag = "h3";
                    break;
                case 4:
                    tag = "h4";
                    break;
                case 5:
                    tag = "h5";
                    break;
                case 6:
                    tag = "h6";
                    break;
                }
                out << "<" << tag << ">" << text << "</" << tag << ">" << endl;
            }else if( f.startsWith("head") )
            {
                const char digit = f[4];
                out << "<h" << digit << ">" << text << "</h" << digit << ">" << endl;
            }else if( comment )
            {
                out << "<blockquote><i>" << text << "</i></blockquote>" << endl;
            }else
            {
                // out << "<u><sup>" << f << "</sup></u>" << endl; // TEST
                text.replace('\t', "&#x0009;");
                out << "<p>" << text << "</p>" << endl;
            }
        }
    }

    void HandleProp(const char *propName, const char *prop, long len)
    {
        // qDebug() << "*** HandleProp" << propName << QByteArray(prop,len);
    }

};

QMap<QByteArray,int> TiogaReader::__formats, TiogaReader::__looks;

TiogaReader::TiogaReader(QObject *parent) : QObject(parent)
{

}

bool TiogaReader::read(const QByteArray& in, const QString& fileName, bool code)
{
    tread_Reader r;
    if( r.init(in.constData(), in.size(), code) )
    {
        r.out.setString(&text,QIODevice::WriteOnly);
        if( !code )
            r.out << "<html>" << endl;
        r.DoWork();
        if( !code )
            r.out << "</html>" << endl;
    }else
        text = tread_Reader::toString(in.constData(),in.size());

    return true;
}

