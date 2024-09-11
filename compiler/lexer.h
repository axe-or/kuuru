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

Token lexer_next(Lexer* lex){
	unimplemented();
}

static
UTF8_Decode_Result lexer_advance(Lexer* lex){
	unimplemented();
}

static
UTF8_Decode_Result lexer_peek(Lexer* lex){
	unimplemented();
}

static
bool advance_on_match(Lexer* lex, Codepoint expect){
	unimplemented();
}

#endif