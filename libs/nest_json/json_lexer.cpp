#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include "json_lexer.h"

#define SET_INVALID_ESCAPE_ERROR \
    Nst_set_syntax_error(Nst_sprintf( \
        "JSON: invalid string escape, file \"%s\", line %lli, column %lli", \
        state.pos.text->path, \
        (i64)state.pos.line, \
        (i64)state.pos.col))

#define SET_INVALID_VALUE_ERROR \
    Nst_set_syntax_error(Nst_sprintf( \
        "JSON: invalid value, file \"%s\", line %lli, column %lli", \
        state.pos.text->path, \
        (i64)state.pos.line, \
        (i64)state.pos.col))

#define IS_HEX(ch) ( (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') )
#define HEX_TO_INT(ch) ( ch <= '9' ? ch - '0' : ch - 'a' + 10 )

#define tok_new_noval(start, end, type) \
    Nst_tok_new_noval(start, end, (Nst_TokType)type)
#define tok_new_noend(start, type) Nst_tok_new_noend(start, (Nst_TokType)type)
#define tok_new_value(start, end, type, value) \
    Nst_tok_new_value(start, end, (Nst_TokType)type, value)

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

static Nst_Tok *parse_json_str();
static Nst_Tok *parse_json_num();
static Nst_Tok *parse_json_val();
static bool ignore_comment();

Nst_LList *json_tokenize(i8   *path,
                         i8   *text,
                         usize text_len,
                         bool  readonly_text)
{
    Nst_SourceText src_text = {
        .text = text,
        .path = path,
        .lines = nullptr,
        .text_len = text_len,
        .lines_len = 0
    };
    Nst_Error error = {
        .occurred = false,
        .start = Nst_no_pos(),
        .end = Nst_no_pos(),
        .name = nullptr,
        .message = nullptr
    };

    if ( readonly_text )
    {
        i8 *text_copy = Nst_malloc_c(text_len, i8);
        if ( text_copy == nullptr )
        {
            return nullptr;
        }
        memcpy(text_copy, text, text_len);
        src_text.text = text_copy;
    }

    bool result = Nst_normalize_encoding(&src_text, Nst_CP_UNKNOWN, &error);
    Nst_add_lines(&src_text);
    if ( error.occurred || !result )
    {
        Nst_set_error(
            error.name,
            error.message);
        Nst_free(src_text.text);
        Nst_free(src_text.lines);
        return nullptr;
    }

    Nst_LList *tokens = Nst_llist_new();
    Nst_Tok *tok = nullptr;

    if ( text_len == 0 )
    {
        goto end;
    }

    state.pos.text = &src_text;
    state.pos.col = 1;
    state.pos.line = 1;
    state.idx = 0;
    state.len = src_text.text_len;
    state.path = path;
    state.ch = *src_text.text;

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
            tok = parse_json_str();
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
            tok = parse_json_num();
            break;
        case 't':
        case 'f':
        case 'n':
            tok = parse_json_val();
            break;
        case '/':
            if ( ignore_comment() )
            {
                continue;
            }
            else
            {
                tok = nullptr;
            }
            break;
        default:
            Nst_set_syntax_error(Nst_sprintf(
                "JSON: invalid character, file \"%s\", line %lli, column %lli",
                path, (i64)state.pos.line, (i64)state.pos.col));
            tok = nullptr;
        }

        if ( tok == nullptr )
        {
            Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_token_destroy);
            Nst_free(src_text.text);
            Nst_free(src_text.lines);
            return nullptr;
        }
        Nst_llist_append(tokens, tok, true);
        advance();
    }
end:
    Nst_llist_append(tokens, tok_new_noend(state.pos, JSON_EOF), true);
    Nst_free(src_text.text);
    Nst_free(src_text.lines);
    return tokens;
}

static Nst_Tok *parse_json_str()
{
    Nst_Pos start = Nst_copy_pos(state.pos);
    Nst_Pos escape_start = Nst_copy_pos(state.pos);
    bool escape = false;

    Nst_Buffer buf;
    if ( !Nst_buffer_init(&buf, 8) )
    {
        return nullptr;
    }

    advance();

    while ( state.idx < state.len && (state.ch != '"' || escape))
    {
        if ( !Nst_buffer_expand_by(&buf, 3) )
        {
            Nst_buffer_destroy(&buf);
            return nullptr;
        }

        if ( !escape )
        {
            if ( (u8)state.ch < ' ' )
            {
                Nst_buffer_destroy(&buf);
                Nst_set_syntax_error(Nst_sprintf(
                    "JSON: invalid character, file \"%s\", line %lli, column %lli",
                    state.pos.text->path,
                    (i64)state.pos.line,
                    (i64)state.pos.col));
                return nullptr;
            }
            else if ( state.ch == '\\' )
            {
                escape = true;
                escape_start = Nst_copy_pos(state.pos);
            }
            else
            {
                Nst_buffer_append_char(&buf, state.ch);
            }
            advance();
            continue;
        }

        // If there is an escape sequence
        switch ( state.ch )
        {
        case '"': Nst_buffer_append_char(&buf, '"' ); break;
        case '\\':Nst_buffer_append_char(&buf, '\\'); break;
        case '/': Nst_buffer_append_char(&buf, '/' ); break;
        case 'b': Nst_buffer_append_char(&buf, '\b'); break;
        case 'f': Nst_buffer_append_char(&buf, '\f'); break;
        case 'n': Nst_buffer_append_char(&buf, '\n'); break;
        case 'r': Nst_buffer_append_char(&buf, '\r'); break;
        case 't': Nst_buffer_append_char(&buf, '\t'); break;
        case 'u':
        {
            advance();
            if ( state.idx + 3 >= state.len )
            {
                Nst_buffer_destroy(&buf);
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
                Nst_buffer_destroy(&buf);
                SET_INVALID_ESCAPE_ERROR;
                return nullptr;
            }

            ch1 = HEX_TO_INT(ch1);
            ch2 = HEX_TO_INT(ch2);
            ch3 = HEX_TO_INT(ch3);
            ch4 = HEX_TO_INT(ch4);

            i32 num = (ch1 << 12) + (ch2 << 8) + (ch3 << 4) + ch4;

            i8 unicode_ch[4] = { 0 };
            Nst_utf8_from_utf32(num, (u8 *)unicode_ch);
            Nst_buffer_append_c_str(&buf, unicode_ch);
            break;
        }
        default:
            Nst_buffer_destroy(&buf);
            SET_INVALID_ESCAPE_ERROR;
            return nullptr;
        }

        escape = false;
        advance();
    }

    if ( state.ch != '"' )
    {
        JSON_SYNTAX_ERROR("open string", state.path, state.pos);
        Nst_buffer_destroy(&buf);
        return nullptr;
    }

    Nst_Obj *val_obj = OBJ(Nst_buffer_to_string(&buf));
    return tok_new_value(start, state.pos, JSON_VALUE, OBJ(val_obj));
}

static Nst_Tok *parse_json_num()
{
    i8 *start_idx = state.pos.text->text + state.idx;
    Nst_Pos start = Nst_copy_pos(state.pos);
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
        i64 value = strtoll(start_idx, nullptr, 10);
        if ( errno == ERANGE )
        {
            Nst_set_memory_error(Nst_sprintf(
                "JSON: number too big, file \"%s\", line %lli, column %lli",
                state.path,
                (i64)state.pos.line,
                (i64)state.pos.col));
            return nullptr;
        }
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            Nst_int_new(value));
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
            Nst_inc_ref(Nst_const()->Int_0));
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
        f64 value = strtod(start_idx, nullptr);
        return tok_new_value(
            start,
            state.pos,
            JSON_VALUE,
            Nst_real_new(value));
    }
}

static Nst_Tok *parse_json_val()
{
    Nst_Pos start = Nst_copy_pos(state.pos);
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
            Nst_inc_ref(Nst_true()));
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
            Nst_inc_ref(Nst_false()));
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
            Nst_inc_ref(Nst_null()));
    }
}

static bool ignore_comment()
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
