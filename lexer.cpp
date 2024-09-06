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

#define TOKEN2(RUNE_, T_, F_) \
    if(advance_on_match(RUNE_)){ \
        tk.kind = T_; \
    } else { \
        tk.kind = F_; \
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
        case '(': {
            tk.kind = TokenKind::Paren_Open;
        } break;

        case '=': TOKEN2('=', K::Eq_Eq, K::Equal);
        case '>': TOKEN2('=', K::Gte, K::Gt);
        case '<': TOKEN2('=', K::Lte, K::Lt);

        case '&': TOKEN2('&', K::Logic_And, K::And);

        case '|': TOKEN3('|', K::Logic_Or, '=', K::Or_Assign, K::Or);
    }

    return tk;
}

#undef TOKEN2
#undef TOKEN3

Lexer Lexer::create(string source){
    Lexer lex;
    lex.source = source;
    lex.iter = source.iter();
    return lex;
}