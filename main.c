#include <stdio.h>
#include <stdlib.h>
#include "lexico.h"
#include "sintatico.h"

int main(void){
   
    const char *src_ok = 
        "program TestePrograma;\n" 
        "var\n"                     
        "  a, b: integer;\n"        
        "begin\n"                   
        "  a := 10 + 5;\n"          
        "  if (a < 20) then\n"      
        "    b := a * 2\n"          
        "  else\n"                  
        "    b := 0;\n"             
        "end.\n";                   

    // Teste com erro sintático (Remoção do ';')
    const char *src_err = 
        "program Erro;\n"           
        "begin\n"                   
        "  a := 10\n"              
        "end.\n";                   

    printf("--- Teste 1: Programa Válido ---\n");
    TokenVec tv_ok = tokenize_to_vector(src_ok);
    parse_program(&tv_ok);
    tv_free(&tv_ok);

    printf("\n--- Teste 2: Programa com Erro (Linha 3: Falta ';') ---\n");
    TokenVec tv_err = tokenize_to_vector(src_err);
    parse_program(&tv_err);
    tv_free(&tv_err);
  
    
    return 0;
}
