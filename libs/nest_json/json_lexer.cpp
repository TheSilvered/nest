#include <cctype>
#include <cerrno>
#include <cstdlib>
#include "json_lexer.h"

#define SET_INVALID_ESCAPE_ERROR \
    NST_SET_SYNTAX_ERROR(nst_format_error( \
        "JSON: invalid string escape, file \"%s\", line %lli, column %lli", \
        "sii", \
        state.pos.text->path, \
        (Nst_Int)state.pos.line, \
        (Nst_Int)state.pos.col))

#define SET_INVALID_VALUE_ERROR \
    NST_SET_SYNTAX_ERROR(nst_format_error( \
        "JSON: invalid value, file \"%s\", line %lli, column %lli", \
        "sii", \
        state.pos.text->path, \
        (Nst_Int)state.pos.line, \
        (Nst_Int)state.pos.col))

#define IS_HEX(ch) ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') )
#define HEX_TO_INT(ch) ( ch <= '9' ? ch - '0' : ch - 'a' + 10 )

#define tok_new_noval(start, end, type) \
    nst_tok_new_noval(start, end, (Nst_TokType)type)
#define tok_new_noend(start, type) nst_tok_new_noend(start, (Nst_TokType)type)
#define tok_new_value(start, end, type, value) \
    nst_tok_new_value(start, end, (Nst_TokType)type, value)

bool comments = false;

typedef struct _LexerState
{
    Nst_Pos pos;
    usize idx;
    usize len;
    i8 *path;
    i8 ch;
}
LexerState;

LexerState state;

static void advance()
{
    state.idx++;
    state.pos.col++;
    if ( state.idx >= state.len )
    {
        state.ch = '\0';
        return;
    }

    state.ch = state.pos.text->text[state.idx];
    if ( state.ch == '\n' )
    {
        state.pos.line++;
        state.pos.col = 0;
    }
}

static void go_back()
{
    state.idx--;
    state.pos.col--;
    state.ch = state.pos.text->text[state.idx];
}

static Nst_Tok *parse_json_str(Nst_OpErr *err);
static Nst_Tok *parse_json_num(Nst_OpErr *err);
static Nst_Tok *parse_json_val(Nst_OpErr *err);
static bool ignore_comment(Nst_OpErr *err);

Nst_LList *json_tokenize(i8        *path,
                         i8        *text,
                         usize      text_len,
                         bool       fix_encoding,
                         Nst_OpErr *err)
{
    Nst_SourceText src_text = {
        .text = text,
        .path = path,
        .len = text_len
    };
    Nst_Error error = {
        .occurred = false
    };

    if ( fix_encoding )
    {
        i32 start_offset = nst_normalize_encoding(&src_text, false, &error);
        nst_add_lines(&src_text, start_offset);
        if ( error.occurred || start_offset == -1 )
        {
            err->name = error.name;
            err->message = error.message;
            return nullptr;
        }
    }

    Nst_LList *tokens = nst_llist_new();
    Nst_Tok *tok = nullptr;

    if ( text_len == 0 )
    {
        goto end;
    }

    state.pos.text = &src_text;
    state.pos.col = 1;
    state.pos.line = 1;
    state.idx = 0;
    state.len = src_text.len;
    state.path = path;
    state.ch = *text;

    while ( state.idx < state.len )
    {
        switch ( state.ch )
        {
        case ' ':
        case '\n':
        case '\r':
        case '\t':
            advance();
            continue;
        case '[':
            tok = tok_new_noval(state.pos, state.pos, JSON_LBRACKET);
            break;
        case ']':
            tok = tok_new_noval(state.pos, state.pos, JSON_RBRACKET);
            break;
        case '{':
            tok = tok_new_noval(state.pos, state.pos, JSON_LBRACE);
            break;
        case '}':
            tok = tok_new_noval(state.pos, state.pos, JSON_RBRACE);
            break;
        case ',':
            tok = tok_new_noval(state.pos, state.pos, JSON_COMMA);
            break;
        case ':':
            tok = tok_new_noval(state.pos, state.pos, JSON_COLON);
            break;
        case '"':
            tok = parse_json_str(err);
            break;
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            tok = parse_json_num(err);
            break;
        case 't':
        case 'f':
        case 'n':
            tok = parse_json_val(err);
            break;
        case '/':
            if ( ignore_comment(err) )
            {
                continue;
            }
            else
            {
                tok = nullptr;
            }
        default:
            NST_SET_SYNTAX_ERROR(nst_format_error(
                "JSON: invalid character, file \"%s\", line %lli, column %lli",
                "sii",
                path, (Nst_Int)state.pos.line, (Nst_Int)state.pos.col));
            tok = nullptr;
        }

        if ( tok == nullptr )
        {
            nst_llist_destroy(tokens, (nst_llist_destructor)nst_token_destroy);
            return nullptr;
        }
        nst_llist_append(tokens, tok, true);
        advance();
    }
end:
    nst_llist_append(tokens, tok_new_noend(state.pos, JSON_EOF), false);
    return tokens;
}

static Nst_Tok *parse_json_str(Nst_OpErr *err)
{
    Nst_Pos start = nst_copy_pos(state.pos);
    Nst_Pos escape_start = nst_copy_pos(state.pos);
    bool escape = false;

    i8 *end_str = (i8 *)malloc(8);
    i8 *end_str_realloc = nullptr;

    if ( end_str == nullptr )
    {
        NST_FAILED_ALLOCATION;
        return nullptr;
    }

    usize str_len = 0;
    usize chunk_size = 8;

    advance();

    while ( state.idx < state.len && (state.ch != '"' || escape))
    {
        if ( str_len + 3 >= chunk_size )
        {
            chunk_size = (usize)(chunk_size * 1.5);
            end_str_realloc = (i8 *)realloc(
                end_str,
                sizeof(i8) * chunk_size);
            if ( end_str_realloc == nullptr )
            {
                free(end_str);
                NST_FAILED_ALLOCATION;
                return nullptr;
            }
            end_str = end_str_realloc;
        }

        if ( !escape )
        {
            if ( (u8)state.ch < ' ' )
            {
                free(end_str);
                NST_SET_SYNTAX_ERROR(nst_format_error(
                    "JSON: invalid character, file \"%s\", line %lli, column %lli",
                    "sii",
                    state.pos.text->path,
                    (Nst_Int)state.pos.line,
                    (Nst_Int)state.pos.col));
                return nullptr;
            }
            else if ( state.ch == '\\' )
            {
                escape = true;
                escape_start = nst_copy_pos(state.pos);
            }
            else
            {
                end_str[str_len++] = state.ch;
            }
            advance();
            continue;
        }

        // If there is an escape sequence
        switch ( state.ch )
        {
        case '"': end_str[str_len++] = '"' ; break;
        case '\\':end_str[str_len++] = '\\'; break;
        case '/': end_str[str_len++] = '/';  break;
        case 'b': end_str[str_len++] = '\b'; break;
        case 'f': end_str[str_len++] = '\f'; break;
        case 'n': end_str[str_len++] = '\n'; break;
        case 'r': end_str[str_len++] = '\r'; break;
        case 't': end_str[str_len++] = '\t'; break;
        case 'u':
        {
            advance();
            if ( state.idx + 3 >= state.len )
            {
                free(end_str);
                SET_INVALID_ESCAPE_ERROR;
                return nullptr;
            }

            i8 ch1 = (i8)tolower(state.ch);
            advance();
            i8 ch2 = (i8)tolower(state.ch);
            advance();
            i8 ch3 = (i8)tolower(state.ch);
            advance();
            i8 ch4 = (i8)tolower(state.ch);

            if ( !IS_HEX(ch1) || !IS_HEX(ch2) || !IS_HEX(ch3) || !IS_HEX(ch4) )
            {
                free(end_str);
                SET_INVALID_ESCAPE_ERROR;
                return nullptr;
            }

            ch1 = HEX_TO_INT(ch1);
            ch2 = HEX_TO_INT(ch2);
            ch3 = HEX_TO_INT(ch3);
            ch4 = HEX_TO_INT(ch4);

            i32 num = (ch1 << 12) + (ch2 << 8) + (ch3 << 4) + ch4;

            if ( num <= 0x7f )
            {
                end_str[str_len++] = (i8)num;
            }
            else if ( num <= 0x7ff )
            {
                end_str[str_len++] = 0b11000000 | (i8)(num >> 6);
                end_str[str_len++] = 0b10000000 | (i8)(num & 0x3f);
            }
            else
            {
                end_str[str_len++] = 0b11100000 | (i8)(num >> 12);
                end_str[str_len++] = 0b10000000 | (i8)(num >> 6 & 0x3f);
                end_str[str_len++] = 0b10000000 | (i8)(num & 0x3f);
            }
            break;
        }
        default:
            free(end_str);
            SET_INVALID_ESCAPE_ERROR;
            return nullptr;
        }

        escape = false;
        advance();
    }

    if ( state.ch != '"' )
    {
        JSON_SYNTAX_ERROR("open string", state.path, state.pos);
        free(end_str);
        return nullptr;
    }

    if ( str_len < chunk_size )
    {
        end_str_realloc = (i8*)realloc(
            end_str,
            sizeof(i8) * (str_len + 1));
    }
    if ( end_str_realloc != nullptr )
    {
        end_str = end_str_realloc;
    }

    end_str[str_len] = '\0';

    Nst_StrObj *val_obj = STR(nst_string_new(end_str, str_len, true));
    nst_obj_hash(OBJ(val_obj));

    return tok_new_value(start, state.pos, JSON_VALUE, OBJ(val_obj));
}

static Nst_Tok *parse_json_num(Nst_OpErr *err)
{
    i8 *start_idx = state.pos.text->text + state.idx;
    Nst_Pos start = nst_copy_pos(state.pos);
    if ( state.ch == '-' )
    {
        advance();
    }

    if ( state.idx >= state.len )
    {
        JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
        return nullptr;
    }
    else if ( state.ch == '0' )
    {
        advance();
        if ( state.ch >= '0' && state.ch <= '9' )
        {
            JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
            return nullptr;
        }

        goto float_ltrl;
    }
    else if ( state.ch > '0' && state.ch <= '9' )
    {
        while ( state.ch >= '0' && state.ch <= '9' )
        {
            advance();
        }

        if ( state.ch == '.' || state.ch == 'e' || state.ch == 'E' )
        {
            goto float_ltrl;
        }
        go_back();
        errno = 0;
        Nst_Int value = strtoll(start_idx, nullptr, 10);
        if ( errno == ERANGE )
        {
            NST_SET_MEMORY_ERROR(nst_format_error(
                "JSON: number too big, file \"%s\", line %lli, column %lli",
                "sii",
                state.path,
                (Nst_Int)state.pos.line,
                (Nst_Int)state.pos.col));
            return nullptr;
        }
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            nst_int_new(value));
    }
    else
    {
        JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
        return nullptr;
    }

float_ltrl:
    if ( state.ch != '.' && state.ch != 'e' && state.ch != 'E' )
    {
        go_back();
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            nst_inc_ref(nst_const()->Int_0));
    }

    if ( state.ch == '.' )
    {
        advance();
        if (state.ch < '0' || state.ch > '9')
        {
            JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
            return nullptr;
        }
        while ( state.ch >= '0' && state.ch <= '9' )
        {
            advance();
        }
    }

    if ( state.ch == 'e' || state.ch == 'E' )
    {
        advance();
        if ( state.ch == '+' || state.ch == '-' )
        {
            advance();
        }

        if ( state.ch < '0' || state.ch > '9' )
        {
            JSON_SYNTAX_ERROR("invalid number", state.path, state.pos);
            return nullptr;
        }
        while ( state.ch >= '0' && state.ch <= '9' )
        {
            advance();
        }
        go_back();
    }
    else
    {
        go_back();
    }

    {
        Nst_Real value = strtod(start_idx, nullptr);
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            nst_real_new(value)); 
    }
}

static Nst_Tok *parse_json_val(Nst_OpErr *err)
{
    Nst_Pos start = nst_copy_pos(state.pos);
    i8 *text = state.pos.text->text + state.idx;
    switch ( state.ch )
    {
    case 't':
        if ( state.idx + 3 >= state.len )
        {
            SET_INVALID_VALUE_ERROR;
            return nullptr;
        }
        if ( text[1] != 'r' || text[2] != 'u' || text[3] != 'e')
        {
            SET_INVALID_VALUE_ERROR;
            return nullptr;
        }
        advance();
        advance();
        advance();
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            nst_inc_ref(nst_true()));
    case 'f':
        if ( state.idx + 4 >= state.len )
        {
            SET_INVALID_VALUE_ERROR;
            return nullptr;
        }

        if ( text[1] != 'a' || text[2] != 'l' ||
             text[3] != 's' || text[4] != 'e' )
        {
            SET_INVALID_VALUE_ERROR;
            return nullptr;
        }
        advance();
        advance();
        advance();
        advance();
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            nst_inc_ref(nst_false()));
    default:
        if ( state.idx + 3 >= state.len )
        {
            SET_INVALID_VALUE_ERROR;
            return nullptr;
        }

        if ( text[1] != 'u' || text[2] != 'l' || text[3] != 'l' )
        {
            SET_INVALID_VALUE_ERROR;
            return nullptr;
        }
        advance();
        advance();
        advance();
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            nst_inc_ref(nst_null()));
    }
}

static bool ignore_comment(Nst_OpErr *err)
{
    if ( !comments )
    {
        JSON_SYNTAX_ERROR("invalid character", state.path, state.pos);
        return false;
    }

    advance();
    if ( state.idx >= state.len || ( state.ch != '/' && state.ch != '*') )
    {
        JSON_SYNTAX_ERROR("invalid character", state.path, state.pos);
        return false;
    }

    bool is_multiline = state.ch == '*';
    bool can_close = false;

    while ( state.idx < state.len )
    {
        advance();
        if ( state.ch == '\n' && !is_multiline )
        {
            advance();
            return true;
        }
        else if ( state.ch == '/' && can_close )
        {
            advance();
            return true;
        }
        else if ( state.ch == '*' )
        {
            can_close = true;
        }
        else
        {
            can_close = false;
        }
    }

    if ( is_multiline )
    {
        JSON_SYNTAX_ERROR("open multiline comment", state.path, state.pos);
        return false;
    }
    return true;
}
