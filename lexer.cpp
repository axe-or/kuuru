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

Token Lexer::next(){
    using K = TokenKind;
    Token tk;

    auto [r, n] = advance();
    if(r == 0){ return Token{ .kind = K::End_Of_File };}

    switch(r){
    }
}

Lexer Lexer::create(string source){
    Lexer lex;
    lex.source = source;
    lex.iter = source.iter();
    return lex;
}