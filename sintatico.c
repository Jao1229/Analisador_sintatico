#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexico.h"

static const char *input;
static int current_line = 1;

/* ---------- Vetor dinâmico (com liberação de lexemas) ---------- */
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
// Função para liberar o vetor, incluindo todos os lexemas alocados
void tv_free(TokenVec *v){ 
    for(int i=0; i<v->size; i++) {
        free(v->data[i].lexeme); 
    }
    free(v->data); v->data=NULL; v->size=v->cap=0; 
}


/* ---------- Lexer (Com Line Counting e Palavras-Chave) ---------- */
static void skip_ws_and_newlines(void){ 
    while(*input==' '||*input=='\t' || *input=='\n'){
        if (*input == '\n') current_line++; // CRUCIAL: Contagem de linha
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
    return ID; // Se não for palavra-chave, é ID
}

static Token getToken(void){
    Token tok = {0, 0.0, NULL, current_line}; // Inicializa linha

    skip_ws_and_newlines();
    tok.line = current_line; // Garante que a linha do token seja a atual após pular espaços

    // Fim de arquivo
    if(*input=='\0'){ tok.type=END_FILE; return tok; }

    // Identificadores e Palavras Reservadas
    if(isalpha((unsigned char)*input)){
        const char *start = input;
        while(isalnum((unsigned char)*input) || *input == '_') input++;
        int len = input - start;
        // strndup não é padrão C99, usaremos strdup ou malloc + memcpy
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

    // Símbolos de 2 ou mais caracteres (CORRIGIDO)
    if (*input == ':') {
        input++;
        if (*input == '=') { tok.type = ASSIGN; input++; tok.lexeme = strdup(":="); } 
        else { tok.type = COLON; tok.lexeme = strdup(":"); }
    }
    else if (*input == '<') {
        input++;
        if (*input == '=') { tok.type = LE; input++; tok.lexeme = strdup("<="); } 
        else if (*input == '>') { tok.type = NE; input++; tok.lexeme = strdup("<>"); } 
        else { tok.type = LT; tok.lexeme = strdup("<"); }
    }
    else if (*input == '>') {
        input++;
        if (*input == '=') { tok.type = GE; input++; tok.lexeme = strdup(">="); } 
        else { tok.type = GT; tok.lexeme = strdup(">"); }
    }
    
    // Símbolos de 1 caractere
    else {
        int char_type = 0;
        switch(*input){
            case '+': char_type = PLUS; break;
            case '-': char_type = MINUS; break;
            case '*': char_type = MULT; break;
            case '/': char_type = DIV; break;
            case '(': char_type = LPAREN; break;
            case ')': char_type = RPAREN; break;
            case ';': char_type = SEMICOLON; break;
            case ',': char_type = COMMA; break;
            case '=': char_type = EQ; break; 
            case '.': char_type = DOT; break;
            default:
                fprintf(stderr,"Erro léxico na linha %d: caractere inesperado '%c'\n", current_line, *input);
                exit(1);
        }
        tok.type = char_type;
        // Aloca o lexema para o caractere simples
        tok.lexeme = (char*)malloc(2);
        tok.lexeme[0] = *input;
        tok.lexeme[1] = '\0';
        input++;
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
        if(t.type == END_FILE) break;
    }
    return v;
}

/* ---------- Nome de token (para mensagens de erro) ---------- */
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
        case END_FILE: return "FIM_DE_ARQUIVO";
        default: return "TOKEN_DESCONHECIDO";
    }
}
