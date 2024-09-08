#include "yuyu.hpp"

pair<rune, i32> Lexer::advance(){
    return iter.next();
}

pair<rune, i32> Lexer::peek(){
    auto [r, n] = iter.next();
    iter.current -= n;
    return {r, n};
}


bool Lexer::advance_on_match(rune expect){
    auto [r, n] = peek();
    if(r == expect){
        iter.current += n;
    }
    return true;
}

#define TOKEN1(T_) \
    { tk.kind = T_; } break

#define TOKEN2(R_, T_, ALT_) \
    if(advance_on_match(R_)){ \
        tk.kind = T_; \
    } else { \
        tk.kind = ALT_; \
    } break

#define TOKEN3(R0_, T0_, R1_, T1_, ALT_) \
    if(advance_on_match(R0_)){ \
        tk.kind = T0_; \
    } else if(advance_on_match(R1_)){ \
        tk.kind = T1_; \
    } else { \
        tk.kind = ALT_; \
    } break

Token Lexer::next(){
    using K = TokenKind;
    Token tk;
    tk.source_offset = iter.current;

    auto [r, n] = advance();
    if(r == 0){ return Token{ .kind = K::End_Of_File };}

    switch(r){
        case '(': TOKEN1(K::Paren_Open);
        case ')': TOKEN1(K::Paren_Close);
        case '[': TOKEN1(K::Square_Open);
        case ']': TOKEN1(K::Square_Close);
        case '{': TOKEN1(K::Curly_Open);
        case '}': TOKEN1(K::Curly_Close);

        case '.': TOKEN1(K::Dot);
        case ',': TOKEN1(K::Comma);
        case ':': TOKEN1(K::Colon);
        case ';': TOKEN1(K::Semicolon);
        case '^': TOKEN1(K::Caret);

        case '!': TOKEN2('=', K::Not_Eq, K::Logic_Not);
        case '=': TOKEN2('=', K::Eq_Eq, K::Equal);
        case '>': TOKEN2('=', K::Gte, K::Gt);
        case '<': TOKEN2('=', K::Lte, K::Lt);

        case '+': TOKEN2('=', K::Plus_Assign, K::Plus);
        case '-': TOKEN2('=', K::Minus_Assign, K::Minus);
        case '*': TOKEN2('=', K::Star_Assign, K::Star);
        case '/': TOKEN2('=', K::Slash_Assign, K::Slash);
        case '%': TOKEN2('=', K::Modulo_Assign, K::Modulo);
         
        case '&': TOKEN3('&', K::Logic_And, '=', K::And_Assign, K::And);
        case '|': TOKEN3('|', K::Logic_Or, '=', K::Or_Assign, K::Or);
        case '~': TOKEN2('=', K::Xor_Assign, K::Xor);
    }

    return tk;
}

#undef TOKEN1
#undef TOKEN2
#undef TOKEN3

Lexer Lexer::create(string source){
    Lexer lex;
    lex.source = source;
    lex.iter = source.iter();
    return lex;
}