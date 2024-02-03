#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "tokens.h"
#include "obj_ops.h"
#include "mem.h"
#include "global_consts.h"
#include "format.h"

#define TOK_TYPE_CASE(tok_name)                                               \
    case Nst_TT_ ## tok_name: Nst_print(#tok_name); break

const i8 *tt_strings[] = {
    [Nst_TT_ADD] = "ADD",
    [Nst_TT_SUB] = "SUB",
    [Nst_TT_MUL] = "MUL",
    [Nst_TT_DIV] = "DIV",
    [Nst_TT_POW] = "POW",
    [Nst_TT_MOD] = "MOD",
    [Nst_TT_B_AND] = "B_AND",
    [Nst_TT_B_OR] = "B_OR",
    [Nst_TT_B_XOR] = "B_XOR",
    [Nst_TT_LSHIFT] = "LSHIFT",
    [Nst_TT_RSHIFT] = "RSHIFT",
    [Nst_TT_CONCAT] = "CONCAT",
    [Nst_TT_L_AND] = "L_AND",
    [Nst_TT_L_OR] = "L_OR",
    [Nst_TT_L_XOR] = "L_XOR",
    [Nst_TT_GT] = "GT",
    [Nst_TT_LT] = "LT",
    [Nst_TT_EQ] = "EQ",
    [Nst_TT_NEQ] = "NEQ",
    [Nst_TT_GTE] = "GTE",
    [Nst_TT_LTE] = "LTE",
    [Nst_TT_CAST] = "CAST",
    [Nst_TT_CALL] = "CALL",
    [Nst_TT_THROW] = "THROW",
    [Nst_TT_RANGE] = "RANGE",
    [Nst_TT_ASSIGN] = "ASSIGN",
    [Nst_TT_ADD_A] = "ADD_A",
    [Nst_TT_SUB_A] = "SUB_A",
    [Nst_TT_MUL_A] = "MUL_A",
    [Nst_TT_DIV_A] = "DIV_A",
    [Nst_TT_POW_A] = "POW_A",
    [Nst_TT_MOD_A] = "MOD_A",
    [Nst_TT_B_AND_A] = "B_AND_A",
    [Nst_TT_B_OR_A] = "B_OR_A",
    [Nst_TT_B_XOR_A] = "B_XOR_A",
    [Nst_TT_LSHIFT_A] = "LSHIFT_A",
    [Nst_TT_RSHIFT_A] = "RSHIFT_A",
    [Nst_TT_CONCAT_A] = "CONCAT_A",
    [Nst_TT_LEN] = "LEN",
    [Nst_TT_L_NOT] = "L_NOT",
    [Nst_TT_B_NOT] = "B_NOT",
    [Nst_TT_STDOUT] = "STDOUT",
    [Nst_TT_STDIN] = "STDIN",
    [Nst_TT_IMPORT] = "IMPORT",
    [Nst_TT_LOC_CALL] = "LOC_CALL",
    [Nst_TT_NEG] = "NEG",
    [Nst_TT_TYPEOF] = "TYPEOF",
    [Nst_TT_IDENT] = "IDENT",
    [Nst_TT_VALUE] = "VALUE",
    [Nst_TT_LAMBDA] = "LAMBDA",
    [Nst_TT_L_PAREN] = "L_PAREN",
    [Nst_TT_L_BRACE] = "L_BRACE",
    [Nst_TT_L_VBRACE] = "L_VBRACE",
    [Nst_TT_L_BRACKET] = "L_BRACKET",
    [Nst_TT_R_PAREN] = "R_PAREN",
    [Nst_TT_R_BRACE] = "R_BRACE",
    [Nst_TT_R_VBRACE] = "R_VBRACE",
    [Nst_TT_R_BRACKET] = "R_BRACKET",
    [Nst_TT_IF] = "IF",
    [Nst_TT_AS] = "AS",
    [Nst_TT_ENDL] = "ENDL",
    [Nst_TT_COMMA] = "COMMA",
    [Nst_TT_COLON] = "COLON",
    [Nst_TT_EOFILE] = "EOFILE",
    [Nst_TT_EXTRACT] = "EXTRACT",
    [Nst_TT_WHILE] = "WHILE",
    [Nst_TT_DOWHILE] = "DOWHILE",
    [Nst_TT_FOR] = "FOR",
    [Nst_TT_FUNC] = "FUNC",
    [Nst_TT_RETURN] = "RETURN",
    [Nst_TT_SWITCH] = "SWITCH",
    [Nst_TT_BREAK] = "BREAK",
    [Nst_TT_CONTINUE] = "CONTINUE",
    [Nst_TT_TRY] = "TRY",
    [Nst_TT_CATCH] = "CATCH",
    [Nst_TT_SEQ_CALL] = "SEQ_CALL",
    [Nst_TT_FMT_STR] = "FMT_STR",
    [Nst_TT_CONTAINS] = "CONTAINS"
};

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

void Nst_tok_destroy(Nst_Tok *token)
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
        if (str[1] == '.' && str[2] == '>')
            return Nst_TT_CONTAINS;
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
    Nst_printf("%s (%02li:%02li, %02li:%02li",
        tt_strings[token->type],
        token->start.line,
        token->start.col,
        token->end.line,
        token->end.col);

    if (token->value != NULL) {
        Nst_print(" - ");

        Nst_StrObj* s = STR(_Nst_repr_str_cast(token->value));
        if (s != NULL) {
            Nst_error_clear();
            Nst_fwrite(s->value, s->len, NULL, Nst_io.out);
            Nst_dec_ref(s);
        }
    }

    Nst_print(")");
}

const i8 *Nst_tok_type_to_str(Nst_TokType type)
{
    return tt_strings[type];
}
