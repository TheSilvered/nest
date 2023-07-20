#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "tokens.h"
#include "obj_ops.h"
#include "mem.h"

Nst_Tok *Nst_tok_new_value(Nst_Pos     start,
                           Nst_Pos     end,
                           Nst_TokType type,
                           Nst_Obj    *value)
{
    Nst_Tok *token = Nst_TOK(Nst_malloc(1, sizeof(Nst_Tok)));
    if ( token == NULL )
    {
        if ( value != NULL )
        {
            Nst_dec_ref(value);
        }
        return NULL;
    }

    token->start = start;
    token->end = end;
    token->value = value;
    token->type = type;

    return token;
}

Nst_Tok *Nst_tok_new_noval(Nst_Pos start, Nst_Pos end, Nst_TokType type)
{
    Nst_Tok *token = Nst_TOK(Nst_malloc(1, sizeof(Nst_Tok)));
    if ( token == NULL )
    {
        return NULL;
    }

    token->start = start;
    token->end = end;
    token->value = NULL;
    token->type = type;

    return token;
}

Nst_Tok *Nst_tok_new_noend(Nst_Pos start, Nst_TokType type)
{
    Nst_Tok *token = Nst_TOK(Nst_malloc(1, sizeof(Nst_Tok)));
    if ( token == NULL )
    {
        return NULL;
    }

    token->start = start;
    token->end = start;
    token->value = NULL;
    token->type = type;

    return token;
}

void Nst_token_destroy(Nst_Tok *token)
{
    if ( token == NULL )
    {
        return;
    }

    if ( token->value != NULL )
    {
        Nst_dec_ref(token->value);
    }
    Nst_free(token);
}

Nst_TokType Nst_tok_from_str(i8 *str)
{
    if ( str[1] == '\0' )
    {
        switch ( str[0] )
        {
        case '+': return Nst_TT_ADD;
        case '-': return Nst_TT_SUB;
        case '*': return Nst_TT_MUL;
        case '/': return Nst_TT_DIV;
        case '^': return Nst_TT_POW;
        case '%': return Nst_TT_MOD;
        case '&': return Nst_TT_B_AND;
        case '|': return Nst_TT_B_OR;
        case '<': return Nst_TT_LT;
        case '>': return Nst_TT_GT;
        case '=': return Nst_TT_ASSIGN;
        case '!': return Nst_TT_L_NOT;
        case '@': return Nst_TT_CALL;
        case '~': return Nst_TT_B_NOT;
        case ':': return Nst_TT_COLON;
        case ';': return Nst_TT_BREAK;
        case '?': return Nst_TT_IF;
        case '.': return Nst_TT_EXTRACT;
        case '#': return Nst_TT_FUNC;
        case '(': return Nst_TT_L_PAREN;
        case ')': return Nst_TT_R_PAREN;
        case '[': return Nst_TT_L_BRACKET;
        case ']': return Nst_TT_R_BRACKET;
        case '{': return Nst_TT_L_BRACE;
        case '}': return Nst_TT_R_BRACE;
        case ',': return Nst_TT_COMMA;
        case '$': return Nst_TT_LEN;
        default: return Nst_TT_INVALID;
        }
    }

    if ( str[2] == '\0' )
    {
        switch ( str[0] )
        {
        case '&':
            if ( str[1] == '&' )
            {
                return Nst_TT_L_AND;
            }
            if ( str[1] == '|' )
            {
                return Nst_TT_L_XOR;
            }
            if ( str[1] == '=' )
            {
                return Nst_TT_B_AND_A;
            }
            break;
        case '|':
            if ( str[1] == '|' )
            {
                return Nst_TT_L_OR;
            }
            if ( str[1] == '=' )
            {
                return Nst_TT_B_OR_A;
            }
            if ( str[1] == '>' )
            {
                return Nst_TT_SWITCH;
            }
            break;
        case '^':
            if ( str[1] == '^' )
            {
                return Nst_TT_B_XOR;
            }
            if ( str[1] == '=' )
            {
                return Nst_TT_POW_A;
            }
            break;
        case '<':
            if ( str[1] == '<' )
            {
                return Nst_TT_LSHIFT;
            }
            if ( str[1] == '=' )
            {
                return Nst_TT_LTE;
            }
            if ( str[1] == '{' )
            {
                return Nst_TT_L_VBRACE;
            }
            break;
        case '>':
            if ( str[1] == '>' )
            {
                return Nst_TT_RSHIFT;
            }
            if ( str[1] == '=' )
            {
                return Nst_TT_GTE;
            }
            if ( str[1] == '<' )
            {
                return Nst_TT_CONCAT;
            }
            break;
        case '=':
            if ( str[1] == '=' )
            {
                return Nst_TT_EQ;
            }
            if ( str[1] == '>' )
            {
                return Nst_TT_RETURN;
            }
            break;
        case '!':
            if ( str[1] == '=' )
            {
                return Nst_TT_NEQ;
            }
            if ( str[1] == '!' )
            {
                return Nst_TT_THROW;
            }
            break;
        case '-':
            if ( str[1] == '>' )
            {
                return Nst_TT_RANGE;
            }
            if ( str[1] == '=' )
            {
                return Nst_TT_SUB_A;
            }
            if ( str[1] == ':' )
            {
                return Nst_TT_NEG;
            }
            break;
        case ':':
            if ( str[1] == ':' )
            {
                return Nst_TT_CAST;
            }
            if ( str[1] == '=' )
            {
                return Nst_TT_AS;
            }
            break;
        case '+':
            if ( str[1] == '=' )
            {
                return Nst_TT_ADD_A;
            }
            break;
        case '*':
            if ( str[1] == '=' )
            {
                return Nst_TT_MUL_A;
            }
            if ( str[1] == '@' )
            {
                return Nst_TT_SEQ_CALL;
            }
            break;
        case '/':
            if ( str[1] == '=' )
            {
                return Nst_TT_DIV_A;
            }
            break;
        case '%':
            if ( str[1] == '=' )
            {
                return Nst_TT_MOD_A;
            }
            break;
        case '@':
            if ( str[1] == '@' )
            {
                return Nst_TT_LOC_CALL;
            }
            break;
        case '.':
            if ( str[1] == '.' )
            {
                return Nst_TT_CONTINUE;
            }
            break;
        case '}':
            if ( str[1] == '>' )
            {
                return Nst_TT_R_VBRACE;
            }
            break;
        case '#':
            if ( str[1] == '#' )
            {
                return Nst_TT_LAMBDA;
            }
            break;
        case '?':
            if ( str[1] == '?' )
            {
                return Nst_TT_TRY;
            }
            if ( str[1] == '!' )
            {
                return Nst_TT_CATCH;
            }
            break;
        }
        return Nst_TT_INVALID;
    }

    if ( str[0] == '>' )
    {
        if ( str[1] == '>' && str[2] == '>' )
        {
            return Nst_TT_STDOUT;
        }
        if ( str[1] == '>' && str[2] == '=' )
        {
            return Nst_TT_RSHIFT_A;
        }
        if ( str[1] == '<' && str[2] == '=' )
        {
            return Nst_TT_CONCAT_A;
        }
    }
    else if ( str[0] == '<' )
    {
        if ( str[1] == '<' && str[2] == '<' )
        {
            return Nst_TT_STDIN;
        }
        if ( str[1] == '<' && str[2] == '=' )
        {
            return Nst_TT_LSHIFT_A;
        }
    }
    else if ( str[0] == '?' )
    {
        if ( str[1] == '.' && str[2] == '.' )
        {
            return Nst_TT_WHILE;
        }
        if ( str[1] == ':' && str[2] == ':' )
        {
            return Nst_TT_TYPEOF;
        }
    }
    else if ( str[0] == '.' )
    {
        if ( str[1] != '.' )
        {
            return Nst_TT_INVALID;
        }
        if ( str[2] == '?' )
        {
            return Nst_TT_DOWHILE;
        }
        if ( str[2] == '.' )
        {
            return Nst_TT_FOR;
        }
    }
    else if ( str[0] == '|' )
    {
        if ( str[1] == '#' && str[2] == '|' )
        {
            return Nst_TT_IMPORT;
        }
    }
    else if ( str[0] == '^' )
    {
        if ( str[1] == '^' && str[2] == '=' )
        {
            return Nst_TT_B_XOR_A;
        }
    }

    return Nst_TT_INVALID;
}

void Nst_print_tok(Nst_Tok *token)
{
    printf(
        "(%02li:%02li, %02li:%02li | ",
        token->start.line,
        token->start.col,
        token->end.line,
        token->end.col);

    switch ( token->type )
    {
    case Nst_TT_ADD:      printf("ADD");      break;
    case Nst_TT_SUB:      printf("SUB");      break;
    case Nst_TT_MUL:      printf("MUL");      break;
    case Nst_TT_DIV:      printf("DIV");      break;
    case Nst_TT_POW:      printf("POW");      break;
    case Nst_TT_MOD:      printf("MOD");      break;
    case Nst_TT_B_AND:    printf("B_AND");    break;
    case Nst_TT_B_OR:     printf("B_OR");     break;
    case Nst_TT_B_XOR:    printf("B_XOR");    break;
    case Nst_TT_LSHIFT:   printf("LSHIFT");   break;
    case Nst_TT_RSHIFT:   printf("RSHIFT");   break;
    case Nst_TT_CONCAT:   printf("CONCAT");   break;
    case Nst_TT_L_AND:    printf("L_AND");    break;
    case Nst_TT_L_OR:     printf("L_OR");     break;
    case Nst_TT_L_XOR:    printf("L_XOR");    break;
    case Nst_TT_GT:       printf("GT");       break;
    case Nst_TT_LT:       printf("LT");       break;
    case Nst_TT_EQ:       printf("EQ");       break;
    case Nst_TT_NEQ:      printf("NEQ");      break;
    case Nst_TT_GTE:      printf("GTE");      break;
    case Nst_TT_LTE:      printf("LTE");      break;
    case Nst_TT_CAST:     printf("CAST");     break;
    case Nst_TT_CALL:     printf("CALL");     break;
    case Nst_TT_THROW:    printf("THROW");    break;
    case Nst_TT_RANGE:    printf("RANGE");    break;
    case Nst_TT_ASSIGN:   printf("ASSIGN");   break;
    case Nst_TT_ADD_A:    printf("ADD_A");    break;
    case Nst_TT_SUB_A:    printf("SUB_A");    break;
    case Nst_TT_MUL_A:    printf("MUL_A");    break;
    case Nst_TT_DIV_A:    printf("DIV_A");    break;
    case Nst_TT_POW_A:    printf("POW_A");    break;
    case Nst_TT_MOD_A:    printf("MOD_A");    break;
    case Nst_TT_B_AND_A:  printf("B_AND_A");  break;
    case Nst_TT_B_OR_A:   printf("B_OR_A");   break;
    case Nst_TT_B_XOR_A:  printf("B_XOR_A");  break;
    case Nst_TT_LSHIFT_A: printf("LSHIFT_A"); break;
    case Nst_TT_RSHIFT_A: printf("RSHIFT_A"); break;
    case Nst_TT_CONCAT_A: printf("CONCAT_A"); break;
    case Nst_TT_LEN:      printf("LEN");      break;
    case Nst_TT_L_NOT:    printf("L_NOT");    break;
    case Nst_TT_B_NOT:    printf("B_NOT");    break;
    case Nst_TT_STDOUT:   printf("STDOUT");   break;
    case Nst_TT_STDIN:    printf("STDIN");    break;
    case Nst_TT_IMPORT:   printf("IMPORT");   break;
    case Nst_TT_LOC_CALL: printf("LOC_CALL"); break;
    case Nst_TT_NEG:      printf("NEG");      break;
    case Nst_TT_TYPEOF:   printf("TYPEOF");   break;
    case Nst_TT_IDENT:    printf("IDENT");    break;
    case Nst_TT_VALUE:    printf("VALUE");    break;
    case Nst_TT_LAMBDA:   printf("LAMBDA");   break;
    case Nst_TT_L_PAREN:  printf("L_PAREN");  break;
    case Nst_TT_L_BRACE:  printf("L_BRACE");  break;
    case Nst_TT_L_VBRACE: printf("L_VBRACE"); break;
    case Nst_TT_L_BRACKET:printf("L_BRACKET");break;
    case Nst_TT_R_PAREN:  printf("R_PAREN");  break;
    case Nst_TT_R_BRACE:  printf("R_BRACE");  break;
    case Nst_TT_R_VBRACE: printf("R_VBRACE"); break;
    case Nst_TT_R_BRACKET:printf("R_BRACKET");break;
    case Nst_TT_IF:       printf("IF");       break;
    case Nst_TT_AS:       printf("AS");       break;
    case Nst_TT_ENDL:     printf("ENDL");     break;
    case Nst_TT_COMMA:    printf("COMMA");    break;
    case Nst_TT_COLON:    printf("COLON");    break;
    case Nst_TT_EOFILE:   printf("EOFILE");   break;
    case Nst_TT_EXTRACT:  printf("EXTRACT");  break;
    case Nst_TT_WHILE:    printf("WHILE");    break;
    case Nst_TT_DOWHILE:  printf("DOWHILE");  break;
    case Nst_TT_FOR:      printf("FOR");      break;
    case Nst_TT_FUNC:     printf("FUNC");     break;
    case Nst_TT_RETURN:   printf("RETURN");   break;
    case Nst_TT_SWITCH:   printf("SWITCH");   break;
    case Nst_TT_BREAK:    printf("BREAK");    break;
    case Nst_TT_CONTINUE: printf("CONTINUE"); break;
    case Nst_TT_TRY:      printf("TRY");      break;
    case Nst_TT_CATCH:    printf("CATCH");    break;
    case Nst_TT_SEQ_CALL: printf("SEQ_CALL"); break;
    default: printf("__UNKNOWN__");
    }

    if ( token->value != NULL )
    {
        printf(" - ");

        Nst_StrObj* s = STR(_Nst_repr_str_cast(token->value));
        if ( s != NULL )
        {
            Nst_error_clear();
            fwrite(s->value, sizeof(i8), s->len, stdout);
            Nst_dec_ref(s);
        }
    }

    printf(")");

    fflush(stdout);
}
