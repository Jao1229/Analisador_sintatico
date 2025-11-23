#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexico.h"

static const char *input;
static int current_line = 1;

/* ---------- Vetor dinâmico (mantido) ---------- */
static void tv_init(TokenVec *v) { v->data=NULL; v->size=v->cap=0; }
static void tv_reserve(TokenVec *v, size_t n){
    if(n <= v->cap) return;
    size_t cap = v->cap ? v->cap : 16;
    while(cap < n) cap *= 2;
    Token *p = realloc(v->data, cap * sizeof(Token));
    if(!p){ perror("realloc"); exit(1); }
    v->data=p; v->cap=cap;
}
static void tv_push(TokenVec *v, Token t){
    if(v->size+1 > v->cap) tv_reserve(v, v->size+1);
    v->data[v->size++] = t;
}
void tv_free(TokenVec *v){ 
    for(int i=0; i<v->size; i++) {
        free(v->data[i].lexeme); // Libera lexema se alocado
    }
    free(v->data); v->data=NULL; v->size=v->cap=0; 
}


/* ---------- Lexer ---------- */
static void skip_ws_and_newlines(void){ 
    while(*input==' '||*input=='\t' || *input=='\n'){
        if (*input == '\n') current_line++;
        input++; 
    }
}
static int check_keyword(const char *s){
    if (strcmp(s, "program") == 0) return PROGRAM_TOK;
    if (strcmp(s, "var") == 0)     return VAR_TOK;
    if (strcmp(s, "integer") == 0) return INTEGER_TOK;
    if (strcmp(s, "real") == 0)    return REAL_TOK;
    if (strcmp(s, "begin") == 0)   return BEGIN_TOK;
    if (strcmp(s, "end") == 0)     return END_TOK;
    if (strcmp(s, "if") == 0)      return IF_TOK;
    if (strcmp(s, "then") == 0)    return THEN_TOK;
    if (strcmp(s, "else") == 0)    return ELSE_TOK;
    if (strcmp(s, "while") == 0)   return WHILE_TOK;
    if (strcmp(s, "do") == 0)      return DO_TOK;
    return ID;
}

static Token getToken(void){
    Token tok = {0, 0.0, NULL, current_line};
    skip_ws_and_newlines();

    // Fim de arquivo
    if(*input=='\0'){ tok.type=END; tok.line=current_line; return tok; }

    // Identificadores e Palavras Reservadas
    if(isalpha((unsigned char)*input)){
        const char *start = input;
        while(isalnum((unsigned char)*input) || *input == '_') input++;
        int len = input - start;
        char *lex = strndup(start, len);
        tok.type = check_keyword(lex);
        tok.lexeme = lex;
        return tok;
    }
    
    // Números
    if(isdigit((unsigned char)*input)){
        char *endptr;
        tok.value = strtod(input, &endptr);
        int len = endptr - input;
        tok.lexeme = strndup(input, len);
        input = endptr;
        tok.type=NUM; 
        return tok;
    }

    // Símbolos de 2 ou mais caracteres
    if (*input == ':') {
        input++;
        if (*input == '=') { tok.type = ASSIGN; input++; } // :=
        else { tok.type = COLON; } // :
    }
    else if (*input == '<') {
        input++;
        if (*input == '=') { tok.type = LE; input++; } // <=
        else if (*input == '>') { tok.type = NE; input++; } // <>
        else { tok.type = LT; } // <
    }
    else if (*input == '>') {
        input++;
        if (*input == '=') { tok.type = GE; input++; } // >=
        else { tok.type = GT; } // >
    }
    
    // Símbolos de 1 caractere
    else {
        switch(*input){
            case '+': tok.type=PLUS; break;
            case '-': tok.type=MINUS; break;
            case '*': tok.type=MULT; break;
            case '/': tok.type=DIV; break;
            case '(': tok.type=LPAREN; break;
            case ')': tok.type=RPAREN; break;
            case ';': tok.type=SEMICOLON; break;
            case ',': tok.type=COMMA; break;
            case '=': tok.type=EQ; break;
            case '.': tok.type=DOT; break;
            default:
                fprintf(stderr,"Erro léxico na linha %d: caractere inesperado '%c'\n", current_line, *input);
                exit(1);
        }
        input++;
    }

    if (!tok.lexeme && tok.type != 0 && tok.type != NUM && tok.type != ID) {
        // Aloca o lexema para o caractere lido (necessário para a função perr)
        tok.lexeme = (char*)malloc(2);
        tok.lexeme[0] = (char)(tok.type);
        tok.lexeme[1] = '\0';
    }

    return tok;
}

/* ---------- Tokenização completa ---------- */
TokenVec tokenize_to_vector(const char *src){
    input = src;
    current_line = 1;
    TokenVec v; tv_init(&v);
    for(;;){
        Token t = getToken();
        tv_push(&v, t);
        if(t.type == END) break;
    }
    return v;
}

/* ---------- Nome de token (atualizado) ---------- */
const char *token_name(int t){
    switch(t){
        case NUM: return "NUMERO";
        case ID: return "IDENTIFICADOR";
        case PROGRAM_TOK: return "PROGRAM";
        case VAR_TOK: return "VAR";
        case INTEGER_TOK: return "INTEGER";
        case REAL_TOK: return "REAL";
        case BEGIN_TOK: return "BEGIN";
        case END_TOK: return "END";
        case IF_TOK: return "IF";
        case THEN_TOK: return "THEN";
        case ELSE_TOK: return "ELSE";
        case WHILE_TOK: return "WHILE";
        case DO_TOK: return "DO";
        case PLUS: return "+";
        case MINUS: return "-";
        case MULT: return "*";
        case DIV: return "/";
        case LPAREN: return "(";
        case RPAREN: return ")";
        case DOT: return ".";
        case SEMICOLON: return ";";
        case COLON: return ":";
        case COMMA: return ",";
        case ASSIGN: return ":=";
        case EQ: return "=";
        case NE: return "<>";
        case LT: return "<";
        case LE: return "<=";
        case GT: return ">";
        case GE: return ">=";
        case END: return "FIM_DE_ARQUIVO";
        default: return "TOKEN_DESCONHECIDO";
    }
}
