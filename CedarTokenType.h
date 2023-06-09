#ifndef __CEDAR_TOKENTYPE__
#define __CEDAR_TOKENTYPE__
// This file was automatically generated by EbnfStudio; don't modify it!


#include <QByteArray>

namespace Cedar {
	enum TokenType {
		Tok_Invalid = 0,

		TT_Literals,
		Tok_Bang,
		Tok_Hash,
		Tok_Lpar,
		Tok_Rpar,
		Tok_Star,
		Tok_Plus,
		Tok_Comma,
		Tok_Minus,
		Tok_2Minus,
		Tok_Dot,
		Tok_2Dot,
		Tok_Slash,
		Tok_Colon,
		Tok_Semi,
		Tok_Lt,
		Tok_2Lt,
		Tok_Leq,
		Tok_Eq,
		Tok_EqGt,
		Tok_Gt,
		Tok_Geq,
		Tok_2Gt,
		Tok_At,
		Tok_Lbrack,
		Tok_Rbrack,
		Tok_Hat,
		Tok_Lbrace,
		Tok_Bar,
		Tok_Rbrace,
		Tok_Tilde,
		Tok_TildeLt,
		Tok_TildeEq,
		Tok_TildeGt,
		Tok_2Tilde,
		Tok_2190,

		TT_Keywords,
		Tok_ABS,
		Tok_ALL,
		Tok_AND,
		Tok_ANY,
		Tok_APPLY,
		Tok_ARRAY,
		Tok_BASE,
		Tok_BEGIN,
		Tok_BROADCAST,
		Tok_CEDAR,
		Tok_CHECKED,
		Tok_CODE,
		Tok_COMPUTED,
		Tok_CONS,
		Tok_CONTINUE,
		Tok_DECREASING,
		Tok_DEFINITIONS,
		Tok_DEPENDENT,
		Tok_DESCRIPTOR,
		Tok_DIRECTORY,
		Tok_DO,
		Tok_ELSE,
		Tok_ENABLE,
		Tok_END,
		Tok_ENDCASE,
		Tok_ENDLOOP,
		Tok_ENTRY,
		Tok_ERROR,
		Tok_EXIT,
		Tok_EXITS,
		Tok_EXPORTS,
		Tok_FINISHED,
		Tok_FIRST,
		Tok_FOR,
		Tok_FORK,
		Tok_FRAME,
		Tok_FREE,
		Tok_FROM,
		Tok_GO,
		Tok_GOTO,
		Tok_IF,
		Tok_IMPORTS,
		Tok_IN,
		Tok_INLINE,
		Tok_INTERNAL,
		Tok_ISTYPE,
		Tok_JOIN,
		Tok_LAST,
		Tok_LENGTH,
		Tok_LIST,
		Tok_LOCKS,
		Tok_LONG,
		Tok_LOOP,
		Tok_LOOPHOLE,
		Tok_MACHINE,
		Tok_MAX,
		Tok_MIN,
		Tok_MOD,
		Tok_MONITOR,
		Tok_MONITORED,
		Tok_NARROW,
		Tok_NEW,
		Tok_NIL,
		Tok_NOT,
		Tok_NOTIFY,
		Tok_NULL,
		Tok_OF,
		Tok_OPEN,
		Tok_OR,
		Tok_ORD,
		Tok_ORDERED,
		Tok_OVERLAID,
		Tok_PACKED,
		Tok_PAINTED,
		Tok_POINTER,
		Tok_PORT,
		Tok_PRED,
		Tok_PRIVATE,
		Tok_PROC,
		Tok_PROCEDURE,
		Tok_PROCESS,
		Tok_PROGRAM,
		Tok_PUBLIC,
		Tok_READONLY,
		Tok_RECORD,
		Tok_REF,
		Tok_REJECT,
		Tok_RELATIVE,
		Tok_REPEAT,
		Tok_RESTART,
		Tok_RESUME,
		Tok_RETRY,
		Tok_RETURN,
		Tok_RETURNS,
		Tok_SAFE,
		Tok_SELECT,
		Tok_SEQUENCE,
		Tok_SHARES,
		Tok_SIGNAL,
		Tok_SIZE,
		Tok_START,
		Tok_STATE,
		Tok_STOP,
		Tok_SUCC,
		Tok_THEN,
		Tok_THROUGH,
		Tok_TO,
		Tok_TRANSFER,
		Tok_TRASH,
		Tok_TRUSTED,
		Tok_TYPE,
		Tok_UNCHECKED,
		Tok_UNCOUNTED,
		Tok_UNSAFE,
		Tok_UNTIL,
		Tok_USING,
		Tok_VAL,
		Tok_VAR,
		Tok_WAIT,
		Tok_WHILE,
		Tok_WITH,
		Tok_ZONE,

		TT_Specials,
		Tok_number,
		Tok_string,
		Tok_char,
		Tok_symbol,
		Tok_n,
		Tok_Comment,
		Tok_Eof,

		TT_MaxToken,

		TT_Max
	};

	const char* tokenTypeString( int ); // Pretty with punctuation chars
	const char* tokenTypeName( int ); // Just the names without punctuation chars
	bool tokenTypeIsLiteral( int );
	bool tokenTypeIsKeyword( int );
	bool tokenTypeIsSpecial( int );
	TokenType tokenTypeFromString( const QByteArray& str, int* pos = 0 );
}
#endif // __CEDAR_TOKENTYPE__
