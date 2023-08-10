#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "tokens.h"
#include "obj_ops.h"
#include "mem.h"
#include "global_consts.h"
#include "format.h"

Nst_Tok *Nst_tok_new_value(Nst_Pos start, Nst_Pos end, Nst_TokType type,
                           Nst_Obj *value)
{
    Nst_Tok *token = Nst_TOK(Nst_malloc(1, sizeof(Nst_Tok)));
    if (token == NULL) {
        Nst_ndec_ref(value);
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
    if (token == NULL)
        return NULL;

    token->start = start;
    token->end = end;
    token->value = NULL;
    token->type = type;

    return token;
}

Nst_Tok *Nst_tok_new_noend(Nst_Pos start, Nst_TokType type)
{
    Nst_Tok *token = Nst_TOK(Nst_malloc(1, sizeof(Nst_Tok)));
    if (token == NULL)
        return NULL;

    token->start = start;
    token->end = start;
    token->value = NULL;
    token->type = type;

    return token;
}

void Nst_token_destroy(Nst_Tok *token)
{
    if (token == NULL)
        return;

    Nst_ndec_ref(token->value);
    Nst_free(token);
}

Nst_TokType Nst_tok_from_str(i8 *str)
{
    if (str[1] == '\0') {
        switch (str[0]) {
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

    if (str[2] == '\0') {
        switch (str[0]) {
        case '&':
            if (str[1] == '&')
                return Nst_TT_L_AND;
            if (str[1] == '|')
                return Nst_TT_L_XOR;
            if (str[1] == '=')
                return Nst_TT_B_AND_A;
            break;
        case '|':
            if (str[1] == '|')
                return Nst_TT_L_OR;
            if (str[1] == '=')
                return Nst_TT_B_OR_A;
            if (str[1] == '>')
                return Nst_TT_SWITCH;
            break;
        case '^':
            if (str[1] == '^')
                return Nst_TT_B_XOR;
            if (str[1] == '=')
                return Nst_TT_POW_A;
            break;
        case '<':
            if (str[1] == '<')
                return Nst_TT_LSHIFT;
            if (str[1] == '=')
                return Nst_TT_LTE;
            if (str[1] == '{')
                return Nst_TT_L_VBRACE;
            break;
        case '>':
            if (str[1] == '>')
                return Nst_TT_RSHIFT;
            if (str[1] == '=')
                return Nst_TT_GTE;
            if (str[1] == '<')
                return Nst_TT_CONCAT;
            break;
        case '=':
            if (str[1] == '=')
                return Nst_TT_EQ;
            if (str[1] == '>')
                return Nst_TT_RETURN;
            break;
        case '!':
            if (str[1] == '=')
                return Nst_TT_NEQ;
            if (str[1] == '!')
                return Nst_TT_THROW;
            break;
        case '-':
            if (str[1] == '>')
                return Nst_TT_RANGE;
            if (str[1] == '=')
                return Nst_TT_SUB_A;
            if (str[1] == ':')
                return Nst_TT_NEG;
            break;
        case ':':
            if (str[1] == ':')
                return Nst_TT_CAST;
            if (str[1] == '=')
                return Nst_TT_AS;
            break;
        case '+':
            if (str[1] == '=')
                return Nst_TT_ADD_A;
            break;
        case '*':
            if (str[1] == '=')
                return Nst_TT_MUL_A;
            if (str[1] == '@')
                return Nst_TT_SEQ_CALL;
            break;
        case '/':
            if (str[1] == '=')
                return Nst_TT_DIV_A;
            break;
        case '%':
            if (str[1] == '=')
                return Nst_TT_MOD_A;
            break;
        case '@':
            if (str[1] == '@')
                return Nst_TT_LOC_CALL;
            break;
        case '.':
            if (str[1] == '.')
                return Nst_TT_CONTINUE;
            break;
        case '}':
            if (str[1] == '>')
                return Nst_TT_R_VBRACE;
            break;
        case '#':
            if (str[1] == '#')
                return Nst_TT_LAMBDA;
            break;
        case '?':
            if (str[1] == '?')
                return Nst_TT_TRY;
            if (str[1] == '!')
                return Nst_TT_CATCH;
            break;
        }
        return Nst_TT_INVALID;
    }

    if (str[0] == '>') {
        if (str[1] == '>' && str[2] == '>')
            return Nst_TT_STDOUT;
        if (str[1] == '>' && str[2] == '=')
            return Nst_TT_RSHIFT_A;
        if (str[1] == '<' && str[2] == '=')
            return Nst_TT_CONCAT_A;
    } else if (str[0] == '<') {
        if (str[1] == '<' && str[2] == '<')
            return Nst_TT_STDIN;
        if (str[1] == '<' && str[2] == '=')
            return Nst_TT_LSHIFT_A;
    } else if (str[0] == '?') {
        if (str[1] == '.' && str[2] == '.')
            return Nst_TT_WHILE;
        if (str[1] == ':' && str[2] == ':')
            return Nst_TT_TYPEOF;
    } else if (str[0] == '.') {
        if (str[1] != '.')
            return Nst_TT_INVALID;
        if (str[2] == '?')
            return Nst_TT_DOWHILE;
        if (str[2] == '.')
            return Nst_TT_FOR;
    } else if (str[0] == '|') {
        if (str[1] == '#' && str[2] == '|')
            return Nst_TT_IMPORT;
    } else if (str[0] == '^') {
        if (str[1] == '^' && str[2] == '=')
            return Nst_TT_B_XOR_A;
    }

    return Nst_TT_INVALID;
}

void Nst_print_tok(Nst_Tok *token)
{
    Nst_printf(
        "(%02li:%02li, %02li:%02li | ",
        token->start.line,
        token->start.col,
        token->end.line,
        token->end.col);

    switch (token->type) {
    case Nst_TT_ADD:      Nst_print("ADD");      break;
    case Nst_TT_SUB:      Nst_print("SUB");      break;
    case Nst_TT_MUL:      Nst_print("MUL");      break;
    case Nst_TT_DIV:      Nst_print("DIV");      break;
    case Nst_TT_POW:      Nst_print("POW");      break;
    case Nst_TT_MOD:      Nst_print("MOD");      break;
    case Nst_TT_B_AND:    Nst_print("B_AND");    break;
    case Nst_TT_B_OR:     Nst_print("B_OR");     break;
    case Nst_TT_B_XOR:    Nst_print("B_XOR");    break;
    case Nst_TT_LSHIFT:   Nst_print("LSHIFT");   break;
    case Nst_TT_RSHIFT:   Nst_print("RSHIFT");   break;
    case Nst_TT_CONCAT:   Nst_print("CONCAT");   break;
    case Nst_TT_L_AND:    Nst_print("L_AND");    break;
    case Nst_TT_L_OR:     Nst_print("L_OR");     break;
    case Nst_TT_L_XOR:    Nst_print("L_XOR");    break;
    case Nst_TT_GT:       Nst_print("GT");       break;
    case Nst_TT_LT:       Nst_print("LT");       break;
    case Nst_TT_EQ:       Nst_print("EQ");       break;
    case Nst_TT_NEQ:      Nst_print("NEQ");      break;
    case Nst_TT_GTE:      Nst_print("GTE");      break;
    case Nst_TT_LTE:      Nst_print("LTE");      break;
    case Nst_TT_CAST:     Nst_print("CAST");     break;
    case Nst_TT_CALL:     Nst_print("CALL");     break;
    case Nst_TT_THROW:    Nst_print("THROW");    break;
    case Nst_TT_RANGE:    Nst_print("RANGE");    break;
    case Nst_TT_ASSIGN:   Nst_print("ASSIGN");   break;
    case Nst_TT_ADD_A:    Nst_print("ADD_A");    break;
    case Nst_TT_SUB_A:    Nst_print("SUB_A");    break;
    case Nst_TT_MUL_A:    Nst_print("MUL_A");    break;
    case Nst_TT_DIV_A:    Nst_print("DIV_A");    break;
    case Nst_TT_POW_A:    Nst_print("POW_A");    break;
    case Nst_TT_MOD_A:    Nst_print("MOD_A");    break;
    case Nst_TT_B_AND_A:  Nst_print("B_AND_A");  break;
    case Nst_TT_B_OR_A:   Nst_print("B_OR_A");   break;
    case Nst_TT_B_XOR_A:  Nst_print("B_XOR_A");  break;
    case Nst_TT_LSHIFT_A: Nst_print("LSHIFT_A"); break;
    case Nst_TT_RSHIFT_A: Nst_print("RSHIFT_A"); break;
    case Nst_TT_CONCAT_A: Nst_print("CONCAT_A"); break;
    case Nst_TT_LEN:      Nst_print("LEN");      break;
    case Nst_TT_L_NOT:    Nst_print("L_NOT");    break;
    case Nst_TT_B_NOT:    Nst_print("B_NOT");    break;
    case Nst_TT_STDOUT:   Nst_print("STDOUT");   break;
    case Nst_TT_STDIN:    Nst_print("STDIN");    break;
    case Nst_TT_IMPORT:   Nst_print("IMPORT");   break;
    case Nst_TT_LOC_CALL: Nst_print("LOC_CALL"); break;
    case Nst_TT_NEG:      Nst_print("NEG");      break;
    case Nst_TT_TYPEOF:   Nst_print("TYPEOF");   break;
    case Nst_TT_IDENT:    Nst_print("IDENT");    break;
    case Nst_TT_VALUE:    Nst_print("VALUE");    break;
    case Nst_TT_LAMBDA:   Nst_print("LAMBDA");   break;
    case Nst_TT_L_PAREN:  Nst_print("L_PAREN");  break;
    case Nst_TT_L_BRACE:  Nst_print("L_BRACE");  break;
    case Nst_TT_L_VBRACE: Nst_print("L_VBRACE"); break;
    case Nst_TT_L_BRACKET:Nst_print("L_BRACKET");break;
    case Nst_TT_R_PAREN:  Nst_print("R_PAREN");  break;
    case Nst_TT_R_BRACE:  Nst_print("R_BRACE");  break;
    case Nst_TT_R_VBRACE: Nst_print("R_VBRACE"); break;
    case Nst_TT_R_BRACKET:Nst_print("R_BRACKET");break;
    case Nst_TT_IF:       Nst_print("IF");       break;
    case Nst_TT_AS:       Nst_print("AS");       break;
    case Nst_TT_ENDL:     Nst_print("ENDL");     break;
    case Nst_TT_COMMA:    Nst_print("COMMA");    break;
    case Nst_TT_COLON:    Nst_print("COLON");    break;
    case Nst_TT_EOFILE:   Nst_print("EOFILE");   break;
    case Nst_TT_EXTRACT:  Nst_print("EXTRACT");  break;
    case Nst_TT_WHILE:    Nst_print("WHILE");    break;
    case Nst_TT_DOWHILE:  Nst_print("DOWHILE");  break;
    case Nst_TT_FOR:      Nst_print("FOR");      break;
    case Nst_TT_FUNC:     Nst_print("FUNC");     break;
    case Nst_TT_RETURN:   Nst_print("RETURN");   break;
    case Nst_TT_SWITCH:   Nst_print("SWITCH");   break;
    case Nst_TT_BREAK:    Nst_print("BREAK");    break;
    case Nst_TT_CONTINUE: Nst_print("CONTINUE"); break;
    case Nst_TT_TRY:      Nst_print("TRY");      break;
    case Nst_TT_CATCH:    Nst_print("CATCH");    break;
    case Nst_TT_SEQ_CALL: Nst_print("SEQ_CALL"); break;
    default: Nst_print("__UNKNOWN__");
    }

    if (token->value != NULL) {
        Nst_print(" - ");

        Nst_StrObj* s = STR(_Nst_repr_str_cast(token->value));
        if (s != NULL) {
            Nst_error_clear();
            Nst_fwrite(s->value, sizeof(i8), s->len, Nst_io.out);
            Nst_dec_ref(s);
        }
    }

    Nst_print(")");

    Nst_fflush(Nst_io.out);
}
