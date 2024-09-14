#pragma once

#include "base.h"
#include "lexer.h"

static bool token_is_literal(TokenKind k){
	static const TokenKind literals[] = {
		Tk_Int, Tk_Real, Tk_Rune, Tk_String, Tk_Nil, Tk_True, Tk_False,
	};
	for(isize i = 0; i < (sizeof(literals) / sizeof(TokenKind)); i += 1){
		if(k == literals[i]){ return true; }
	}
	return false;
}

static bool token_is_keyword(TokenKind k){
	static const TokenKind keywords[] = {
		#define X(Name, Str) Tk_##Name,
		KUURU_KEYWORD_TABLE
		#undef X
	};
	for(isize i = 0; i < (sizeof(keywords) / sizeof(TokenKind)); i += 1){
		if(k == keywords[i]){ return true; }
	}
	return false;
}

static void format_token_list(Bytes_Buffer* bb, Token* tokens, isize count){
	static const cstring keyword_to_str[] = {
		#define X(Name, Str) [Tk_##Name] = Str,
		KUURU_KEYWORD_TABLE
		#undef X
	};

	static const cstring lit_to_str[] = {
		[Tk_Int] = "Int:",
		[Tk_Real] = "Real:",
		[Tk_String] = "String:",
		[Tk_Rune] = "Rune:",
	};

	for(isize i = 0; i < count; i += 1){
		Token tk = tokens[i];
		if(tk.kind == Tk_Identifier){
			buffer_write(bb, tk.lexeme.data, tk.lexeme.len);
		}
		else if(token_is_keyword(tk.kind)){
			cstring kw = keyword_to_str[tk.kind];
			buffer_write(bb, (byte*)(kw), cstring_len(kw));
		}
		else if(token_is_literal(tk.kind)){
			cstring prefix = lit_to_str[tk.kind];
			buffer_write(bb, (byte*)(prefix), cstring_len(prefix));
			buffer_write(bb, tk.lexeme.data, tk.lexeme.len);
		}
		buffer_write(bb, (byte*)(" "), 1);
	}
}

