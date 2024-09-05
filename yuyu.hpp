#pragma once

#include "base/base.hpp"

enum class TokenKind : i8 {
	Unknown = 0,

	Identifier, Let, Fn, Struct, If, Else, For, Match, Break, Continue, Return,

	Int, Real, String, Rune, True, False, Nil,

	Paren_Open,	Paren_Close,
	Square_Open, Square_Close,
	Curly_Open,	Curly_Close,

	Dot, Comma, Colon, Semicolon, Equal, Caret,

	Plus, Minus, Star, Slash, Modulo,
	And, Or, Xor,

	Logic_And, Logic_Or, Logic_Not,

	Gt, Gte, Lt, Lte, Eq_Eq, Not_Eq,

	End_Of_File,

	_enum_length,
};

struct Token {
	TokenKind kind{0};
	string lexeme = "";
	i32 source_offset = 0;
};

struct Lexer {
	string source = "";
	utf8::Iterator iter;
	
    auto done() { return iter.done(); }

	pair<rune, i32> advance();
	pair<rune, i32> peek();
	bool advance_on_match(rune expect);
	Token next();

	static Lexer create(string source);
};