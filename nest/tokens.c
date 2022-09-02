#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "nst_types.h"
#include "tokens.h"
#include "obj_ops.h"

Nst_LexerToken *nst_new_token_value(Nst_Pos start,
                                    Nst_Pos end,
                                    int type,
                                    Nst_Obj *value)
{
    Nst_LexerToken *token = malloc(sizeof(Nst_LexerToken));
    if ( token == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    token->start = start;
    token->end = end;
    token->value = value;
    token->type = type;

    return token;
}

Nst_LexerToken *nst_new_token_noval(Nst_Pos start, Nst_Pos end, int type)
{
    Nst_LexerToken *token = malloc(sizeof(Nst_LexerToken));
    if ( token == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    token->start = start;
    token->end = end;
    token->value = NULL;
    token->type = type;

    return token;
}

Nst_LexerToken *nst_new_token_noend(Nst_Pos start, int type)
{
    Nst_LexerToken *token = malloc(sizeof(Nst_LexerToken));
    if ( token == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    token->start = start;
    token->end = start;
    token->value = NULL;
    token->type = type;

    return token;
}

void nst_destroy_token(Nst_LexerToken *token)
{
    if ( token == NULL )
        return;

    if ( token->value != NULL )
        nst_dec_ref(token->value);
    free(token);
}

int nst_str_to_tok(char *str)
{
    if ( strcmp("+", str) == 0 )
        return NST_TT_ADD;
    if ( strcmp("-", str) == 0 )
        return NST_TT_SUB;
    if ( strcmp("*", str) == 0 )
        return NST_TT_MUL;
    if ( strcmp("/", str) == 0 )
        return NST_TT_DIV;
    if ( strcmp("^", str) == 0 )
        return NST_TT_POW;
    if ( strcmp("%", str) == 0 )
        return NST_TT_MOD;
    if ( strcmp("&&", str) == 0 )
        return NST_TT_L_AND;
    if ( strcmp("||", str) == 0 )
        return NST_TT_L_OR;
    if ( strcmp("&|", str) == 0 )
        return NST_TT_L_XOR;
    if ( strcmp("&", str) == 0 )
        return NST_TT_B_AND;
    if ( strcmp("|", str) == 0 )
        return NST_TT_B_OR;
    if ( strcmp("^^", str) == 0 )
        return NST_TT_B_XOR;
    if ( strcmp("<<", str) == 0 )
        return NST_TT_LSHIFT;
    if ( strcmp(">>", str) == 0 )
        return NST_TT_RSHIFT;
    if ( strcmp(">", str) == 0 )
        return NST_TT_GT;
    if ( strcmp("<", str) == 0 )
        return NST_TT_LT;
    if ( strcmp("==", str) == 0 )
        return NST_TT_EQ;
    if ( strcmp("!=", str) == 0 )
        return NST_TT_NEQ;
    if ( strcmp(">=", str) == 0 )
        return NST_TT_GTE;
    if ( strcmp("<=", str) == 0 )
        return NST_TT_LTE;
    if ( strcmp("><", str) == 0 )
        return NST_TT_CONCAT;
    if ( strcmp(">>>", str) == 0 )
        return NST_TT_STDOUT;
    if ( strcmp("<<<", str) == 0 )
        return NST_TT_STDIN;
    if ( strcmp("->", str) == 0 )
        return NST_TT_RANGE;
    if ( strcmp("$", str) == 0 )
        return NST_TT_LEN;
    if ( strcmp("::", str) == 0 )
        return NST_TT_CAST;
    if ( strcmp("?::", str) == 0 )
        return NST_TT_TYPEOF;
    if ( strcmp("=", str) == 0 )
        return NST_TT_ASSIGN;
    if ( strcmp("+=", str) == 0 )
        return NST_TT_ADD_A;
    if ( strcmp("-=", str) == 0 )
        return NST_TT_SUB_A;
    if ( strcmp("*=", str) == 0 )
        return NST_TT_MUL_A;
    if ( strcmp("/=", str) == 0 )
        return NST_TT_DIV_A;
    if ( strcmp("^=", str) == 0 )
        return NST_TT_POW_A;
    if ( strcmp("%=", str) == 0 )
        return NST_TT_MOD_A;
    if ( strcmp("|=", str) == 0 )
        return NST_TT_B_OR_A;
    if ( strcmp("&=", str) == 0 )
        return NST_TT_B_AND_A;
    if ( strcmp("^^=", str) == 0 )
        return NST_TT_B_XOR_A;
    if ( strcmp("<<=", str) == 0 )
        return NST_TT_LSHIFT_A;
    if ( strcmp(">>=", str) == 0 )
        return NST_TT_RSHIFT_A;
    if ( strcmp("><=", str) == 0 )
        return NST_TT_CONCAT_A;
    if ( strcmp("@", str) == 0 )
        return NST_TT_CALL;
    if ( strcmp("@@", str) == 0 )
        return NST_TT_LOC_CALL;
    if ( strcmp("!", str) == 0 )
        return NST_TT_L_NOT;
    if ( strcmp("~", str) == 0 )
        return NST_TT_B_NOT;
    if ( strcmp(".", str) == 0 )
        return NST_TT_EXTRACT;
    if ( strcmp("-:", str) == 0 )
        return NST_TT_NEG;
    if ( strcmp("?", str) == 0 )
        return NST_TT_IF;
    if ( strcmp("?..", str) == 0 )
        return NST_TT_WHILE;
    if ( strcmp("..?", str) == 0 )
        return NST_TT_DOWHILE;
    if ( strcmp("...", str) == 0 )
        return NST_TT_FOR;
    if ( strcmp("~=", str) == 0 )
        return NST_TT_AS;
    if ( strcmp("#", str) == 0 )
        return NST_TT_FUNC;
    if ( strcmp("=>", str) == 0 )
        return NST_TT_RETURN;
    if ( strcmp("|>", str) == 0 )
        return NST_TT_SWITCH;
    if ( strcmp("..", str) == 0 )
        return NST_TT_CONTINUE;
    if ( strcmp(";", str) == 0 )
        return NST_TT_BREAK;
    if ( strcmp(":", str) == 0 )
        return NST_TT_COLON;
    if ( strcmp("|#|", str) == 0 )
        return NST_TT_IMPORT;
    if ( strcmp("$", str) == 0 )
        return NST_TT_LEN;
    if ( strcmp(",", str) == 0 )
        return NST_TT_COMMA;
    if ( strcmp("(", str) == 0 )
        return NST_TT_L_PAREN;
    if ( strcmp(")", str) == 0 )
        return NST_TT_R_PAREN;
    if ( strcmp("[", str) == 0 )
        return NST_TT_L_BRACKET;
    if ( strcmp("]", str) == 0 )
        return NST_TT_R_BRACKET;
    if ( strcmp("{", str) == 0 )
        return NST_TT_L_BRACE;
    if ( strcmp("}", str) == 0 )
        return NST_TT_R_BRACE;
    if ( strcmp("<{", str) == 0 )
        return NST_TT_L_VBRACE;
    if ( strcmp("}>", str) == 0 )
        return NST_TT_R_VBRACE;
    return -1;
}

void nst_print_token(Nst_LexerToken *token)
{
    printf("(%02zi:%02zi, %02zi:%02zi | ",
        token->start.line,
        token->start.col,
        token->end.line,
        token->end.col
    );

    switch ( token->type )
    {
    case NST_TT_ADD:      printf("ADD");      break;
    case NST_TT_SUB:      printf("SUB");      break;
    case NST_TT_MUL:      printf("MUL");      break;
    case NST_TT_DIV:      printf("DIV");      break;
    case NST_TT_POW:      printf("POW");      break;
    case NST_TT_MOD:      printf("MOD");      break;
    case NST_TT_B_AND:    printf("B_AND");    break;
    case NST_TT_B_OR:     printf("B_OR");     break;
    case NST_TT_B_XOR:    printf("B_XOR");    break;
    case NST_TT_LSHIFT:   printf("LSHIFT");   break;
    case NST_TT_RSHIFT:   printf("RSHIFT");   break;
    case NST_TT_CONCAT:   printf("CONCAT");   break;
    case NST_TT_L_AND:    printf("L_AND");    break;
    case NST_TT_L_OR:     printf("L_OR");     break;
    case NST_TT_L_XOR:    printf("L_XOR");    break;
    case NST_TT_GT:       printf("GT");       break;
    case NST_TT_LT:       printf("LT");       break;
    case NST_TT_EQ:       printf("EQ");       break;
    case NST_TT_NEQ:      printf("NEQ");      break;
    case NST_TT_GTE:      printf("GTE");      break;
    case NST_TT_LTE:      printf("LTE");      break;
    case NST_TT_CAST:     printf("CAST");     break;
    case NST_TT_CALL:     printf("CALL");     break;
    case NST_TT_RANGE:    printf("RANGE");    break;
    case NST_TT_ASSIGN:   printf("ASSIGN");   break;
    case NST_TT_ADD_A:    printf("ADD_A");    break;
    case NST_TT_SUB_A:    printf("SUB_A");    break;
    case NST_TT_MUL_A:    printf("MUL_A");    break;
    case NST_TT_DIV_A:    printf("DIV_A");    break;
    case NST_TT_POW_A:    printf("POW_A");    break;
    case NST_TT_MOD_A:    printf("MOD_A");    break;
    case NST_TT_B_AND_A:  printf("B_AND_A");  break;
    case NST_TT_B_OR_A:   printf("B_OR_A");   break;
    case NST_TT_B_XOR_A:  printf("B_XOR_A");  break;
    case NST_TT_LSHIFT_A: printf("LSHIFT_A"); break;
    case NST_TT_RSHIFT_A: printf("RSHIFT_A"); break;
    case NST_TT_CONCAT_A: printf("CONCAT_A"); break;
    case NST_TT_LEN:      printf("LEN");      break;
    case NST_TT_L_NOT:    printf("L_NOT");    break;
    case NST_TT_B_NOT:    printf("B_NOT");    break;
    case NST_TT_STDOUT:   printf("STDOUT");   break;
    case NST_TT_STDIN:    printf("STDIN");    break;
    case NST_TT_IMPORT:   printf("IMPORT");   break;
    case NST_TT_LOC_CALL: printf("LOC_CALL"); break;
    case NST_TT_NEG:      printf("NEG");      break;
    case NST_TT_TYPEOF:   printf("TYPEOF");   break;
    case NST_TT_IDENT:    printf("IDENT");    break;
    case NST_TT_INT:      printf("INT");      break;
    case NST_TT_REAL:     printf("REAL");     break;
    case NST_TT_STRING:   printf("STRING");   break;
    case NST_TT_L_PAREN:  printf("L_PAREN");  break;
    case NST_TT_L_BRACE:  printf("L_BRACE");  break;
    case NST_TT_L_VBRACE: printf("L_VBRACE"); break;
    case NST_TT_L_BRACKET:printf("L_BRACKET");break;
    case NST_TT_R_PAREN:  printf("R_PAREN");  break;
    case NST_TT_R_BRACE:  printf("R_BRACE");  break;
    case NST_TT_R_VBRACE: printf("R_VBRACE"); break;
    case NST_TT_R_BRACKET:printf("R_BRACKET");break;
    case NST_TT_IF:       printf("IF");       break;
    case NST_TT_AS:       printf("AS");       break;
    case NST_TT_ENDL:     printf("ENDL");     break;
    case NST_TT_COMMA:    printf("COMMA");    break;
    case NST_TT_COLON:    printf("COLON");    break;
    case NST_TT_EOFILE:   printf("EOFILE");   break;
    case NST_TT_EXTRACT:  printf("EXTRACT");  break;
    case NST_TT_WHILE:    printf("WHILE");    break;
    case NST_TT_DOWHILE:  printf("DOWHILE");  break;
    case NST_TT_FOR:      printf("FOR");      break;
    case NST_TT_FUNC:     printf("FUNC");     break;
    case NST_TT_RETURN:   printf("RETURN");   break;
    case NST_TT_SWITCH:   printf("SWITCH");   break;
    case NST_TT_BREAK:    printf("BREAK");    break;
    case NST_TT_CONTINUE: printf("CONTINUE"); break;
    default: printf("__UNKNOWN__");
    }

    if ( token->value != NULL )
    {
        printf(" - ");

        if ( token->type == NST_TT_STRING || token->type == NST_TT_IDENT )
        {
            Nst_StrObj *s = AS_STR(_nst_repr_string(AS_STR(token->value)));
            printf("%s", s->value);
            nst_dec_ref(s);
        }
        else
            nst_obj_stdout(token->value, NULL);
    }

    printf(")");

    fflush(stdout);
}
