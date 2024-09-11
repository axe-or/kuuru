#pragma once

#include "base/assert.h"
#include "base/utf8.h"
#include "base/string.h"

/// Interface //////////////////////////////////////////////////////////////////
typedef enum TokenKind TokenKind;
typedef struct Token Token;
typedef struct Lexer Lexer;

enum TokenKind {
	Tk_Unknown = 0,

	Tk_Identifier, Tk_Let, Tk_Fn, Tk_Struct, Tk_If, Tk_Else, Tk_For, Tk_Match, Tk_Break, Tk_Continue, Tk_Return,

	Tk_Int, Tk_Real, Tk_String, Tk_Rune, Tk_True, Tk_False, Tk_Nil,

	Tk_Paren_Open,	Tk_Paren_Close, Tk_Square_Open, Tk_Square_Close, Tk_Curly_Open,	Tk_Curly_Close,

	Tk_Dot, Tk_Comma, Tk_Colon, Tk_Semicolon, Tk_Equal, Tk_Caret,

	Tk_Plus, Tk_Minus, Tk_Star, Tk_Slash, Tk_Modulo,
	Tk_And, Tk_Or, Tk_Xor,

	Tk_Logic_And, Tk_Logic_Or, Tk_Logic_Not,

	Tk_Gt, Tk_Gte, Tk_Lt, Tk_Lte, Tk_Eq_Eq, Tk_Not_Eq,

	// Assignment versions of operators
	Tk_Plus_Assign, Tk_Minus_Assign, Tk_Star_Assign, Tk_Slash_Assign, Tk_Modulo_Assign,
	Tk_And_Assign, Tk_Or_Assign, Tk_Xor_Assign,

	Tk_EOF,

	// _tk_enum_length,
};

struct Token {
	TokenKind kind;
	String lexeme;
	isize source_offset;
};

struct Lexer {
	String source;
	UTF8_Iterator iter;
};

// Create and initialize a lexer from a source
Lexer lexer_make(String source);
// Is lexer finished reading its source?
bool lexer_done(Lexer const* lex);
// Get next token. Returns an End_Of_File token when finished.
Token lexer_next(Lexer* lex);

/// Implementation /////////////////////////////////////////////////////////////
#ifdef KUURU_IMPLEMENTATION
Lexer lexer_make(String source){
	UTF8_Iterator iterator = {
		.current = 0,
		.data = source.data,
		.data_length = source.len,
	};
	return (Lexer){
		.source = source,
		.iter = iterator,
	};
}
bool lexer_done(Lexer const* lex){
	return lex->iter.current >= lex->iter.data_length;
}


static
UTF8_Decode_Result lexer_advance(Lexer* lex){
	UTF8_Decode_Result res = { 0, 0 };
	Codepoint c; i8 len;
	bool ok = utf8_iter_next(&lex->iter, &c, &len);

	if(ok){
		res.codepoint = c;
		res.len = len;
	}
	return res;
}

static
UTF8_Decode_Result lexer_peek(Lexer* lex){
	UTF8_Decode_Result res = lexer_advance(lex);
	lex->iter.current -= res.len;
	return res;
}

static
bool lexer_advance_on_match(Lexer* lex, Codepoint expect){
	UTF8_Decode_Result res = lexer_peek(lex);
	unimplemented();
}

#define TOKEN1(T_) \
    { tk.kind = T_; } break

#define TOKEN2(R_, T_, ALT_) \
    if(lexer_advance_on_match(lexer, R_)){ \
        tk.kind = T_; \
    } else { \
        tk.kind = ALT_; \
    } break

#define TOKEN3(R0_, T0_, R1_, T1_, ALT_) \
    if(lexer_advance_on_match(lexer, R0_)){ \
        tk.kind = T0_; \
    } else if(lexer_advance_on_match(lexer, R1_)){ \
        tk.kind = T1_; \
    } else { \
        tk.kind = ALT_; \
    } break


Token lexer_next(Lexer* lexer){
	Token tk = {0};
	tk.source_offset = lexer->iter.current;

	UTF8_Decode_Result res = lexer_advance(lexer);
	if(res.codepoint == 0){
		tk.kind = Tk_EOF;
		return tk;
	}

	switch(res.codepoint){
		case '(': TOKEN1(Tk_Paren_Open);
		case ')': TOKEN1(Tk_Paren_Close);
		case '[': TOKEN1(Tk_Square_Open);
		case ']': TOKEN1(Tk_Square_Close);
		case '{': TOKEN1(Tk_Curly_Open);
		case '}': TOKEN1(Tk_Curly_Close);

		case '.': TOKEN1(Tk_Dot);
		case ',': TOKEN1(Tk_Comma);
		case ':': TOKEN1(Tk_Colon);
		case ';': TOKEN1(Tk_Semicolon);
		case '^': TOKEN1(Tk_Caret);

		case '!': TOKEN2('=', Tk_Not_Eq, Tk_Logic_Not);

        case '=': TOKEN2('=', Tk_Eq_Eq, Tk_Equal);
        case '>': TOKEN2('=', Tk_Gte, Tk_Gt);
        case '<': TOKEN2('=', Tk_Lte, Tk_Lt);

        case '+': TOKEN2('=', Tk_Plus_Assign, Tk_Plus);
        case '-': TOKEN2('=', Tk_Minus_Assign, Tk_Minus);
        case '*': TOKEN2('=', Tk_Star_Assign, Tk_Star);
        case '/': TOKEN2('=', Tk_Slash_Assign, Tk_Slash);
        case '%': TOKEN2('=', Tk_Modulo_Assign, Tk_Modulo);
         
        case '&': TOKEN3('&', Tk_Logic_And, '=', Tk_And_Assign, Tk_And);
        case '|': TOKEN3('|', Tk_Logic_Or, '=', Tk_Or_Assign, Tk_Or);
        case '~': TOKEN2('=', Tk_Xor_Assign, Tk_Xor);

		case '\t': case ' ': case '\r': break; /* Ignore whitespace */

		case '\n': /* TODO: Auto EOS insertion */ break;
		default: {} break;
	}

	return tk;
}

#undef TOKEN1
#undef TOKEN2
#undef TOKEN3
#endif