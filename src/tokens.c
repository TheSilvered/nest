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
    Nst_printf(
        "(%02li:%02li, %02li:%02li | ",
        token->start.line,
        token->start.col,
        token->end.line,
        token->end.col);

    switch (token->type) {
        TOK_TYPE_CASE(ADD);
        TOK_TYPE_CASE(SUB);
        TOK_TYPE_CASE(MUL);
        TOK_TYPE_CASE(DIV);
        TOK_TYPE_CASE(POW);
        TOK_TYPE_CASE(MOD);
        TOK_TYPE_CASE(B_AND);
        TOK_TYPE_CASE(B_OR);
        TOK_TYPE_CASE(B_XOR);
        TOK_TYPE_CASE(LSHIFT);
        TOK_TYPE_CASE(RSHIFT);
        TOK_TYPE_CASE(CONCAT);
        TOK_TYPE_CASE(L_AND);
        TOK_TYPE_CASE(L_OR);
        TOK_TYPE_CASE(L_XOR);
        TOK_TYPE_CASE(GT);
        TOK_TYPE_CASE(LT);
        TOK_TYPE_CASE(EQ);
        TOK_TYPE_CASE(NEQ);
        TOK_TYPE_CASE(GTE);
        TOK_TYPE_CASE(LTE);
        TOK_TYPE_CASE(CAST);
        TOK_TYPE_CASE(CALL);
        TOK_TYPE_CASE(THROW);
        TOK_TYPE_CASE(RANGE);
        TOK_TYPE_CASE(ASSIGN);
        TOK_TYPE_CASE(ADD_A);
        TOK_TYPE_CASE(SUB_A);
        TOK_TYPE_CASE(MUL_A);
        TOK_TYPE_CASE(DIV_A);
        TOK_TYPE_CASE(POW_A);
        TOK_TYPE_CASE(MOD_A);
        TOK_TYPE_CASE(B_AND_A);
        TOK_TYPE_CASE(B_OR_A);
        TOK_TYPE_CASE(B_XOR_A);
        TOK_TYPE_CASE(LSHIFT_A);
        TOK_TYPE_CASE(RSHIFT_A);
        TOK_TYPE_CASE(CONCAT_A);
        TOK_TYPE_CASE(LEN);
        TOK_TYPE_CASE(L_NOT);
        TOK_TYPE_CASE(B_NOT);
        TOK_TYPE_CASE(STDOUT);
        TOK_TYPE_CASE(STDIN);
        TOK_TYPE_CASE(IMPORT);
        TOK_TYPE_CASE(LOC_CALL);
        TOK_TYPE_CASE(NEG);
        TOK_TYPE_CASE(TYPEOF);
        TOK_TYPE_CASE(IDENT);
        TOK_TYPE_CASE(VALUE);
        TOK_TYPE_CASE(LAMBDA);
        TOK_TYPE_CASE(L_PAREN);
        TOK_TYPE_CASE(L_BRACE);
        TOK_TYPE_CASE(L_VBRACE);
        TOK_TYPE_CASE(L_BRACKET);
        TOK_TYPE_CASE(R_PAREN);
        TOK_TYPE_CASE(R_BRACE);
        TOK_TYPE_CASE(R_VBRACE);
        TOK_TYPE_CASE(R_BRACKET);
        TOK_TYPE_CASE(IF);
        TOK_TYPE_CASE(AS);
        TOK_TYPE_CASE(ENDL);
        TOK_TYPE_CASE(COMMA);
        TOK_TYPE_CASE(COLON);
        TOK_TYPE_CASE(EOFILE);
        TOK_TYPE_CASE(EXTRACT);
        TOK_TYPE_CASE(WHILE);
        TOK_TYPE_CASE(DOWHILE);
        TOK_TYPE_CASE(FOR);
        TOK_TYPE_CASE(FUNC);
        TOK_TYPE_CASE(RETURN);
        TOK_TYPE_CASE(SWITCH);
        TOK_TYPE_CASE(BREAK);
        TOK_TYPE_CASE(CONTINUE);
        TOK_TYPE_CASE(TRY);
        TOK_TYPE_CASE(CATCH);
        TOK_TYPE_CASE(SEQ_CALL);
        TOK_TYPE_CASE(FMT_STR);
        TOK_TYPE_CASE(CONTAINS);
        default: Nst_print("__UNKNOWN__");
    }

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

    Nst_fflush(Nst_io.out);
}
