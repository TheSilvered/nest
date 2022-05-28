#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "error.h"

Pos copy_pos(Pos pos)
{
    Pos new_pos = {
        pos.line,
        pos.col,
        pos.filename,
        pos.text,
        pos.text_len
    };

    return new_pos;
}

typedef struct PrintLineResult
{
    size_t len;
    size_t indent;
}
PrintLineResult;

PrintLineResult print_line(size_t lineno, char *text, size_t text_len)
{
    PrintLineResult result = { 0, 0 };
    size_t curr_line = 0;
    bool is_indentation = true;

    for ( int i = 0; i < text_len; i++ )
    {
        if ( curr_line == lineno )
        {
            if ( text[i] != ' ' && text[i] != '\t' )
                is_indentation = false;

            if ( !is_indentation )
            {
                printf("%c", text[i]);
                result.len++;
            }
            else
                result.indent++;
        }

        if ( text[i] == '\n' )
        {
            curr_line++;
            if ( curr_line - 1 == lineno )
                break;
        }
    }

    if ( curr_line == lineno )
        printf("\n");
    else
        result.len--;
    return result;
}

inline void print_repeat(char ch, size_t times)
{
    if ( times < 0 ) times = 1;
    for ( int i = 0; i < times; i++ )
        printf("%c", ch);
}

void print_position(Pos start, Pos end)
{
    assert(start.filename == end.filename);
    printf("File \"%s\" at line %lli", start.filename, start.line + 1);
    if ( start.line != end.line ) printf(" to %lli", end.line);
    printf(":\n| ");

    assert(start.text == end.text);

    PrintLineResult res = print_line(start.line, start.text, start.text_len);
    printf("| ");
    print_repeat(' ', start.col - res.indent);

    if ( start.line == end.line )
    {
        print_repeat('^', end.col - start.col + 1);
        printf("\n");
        return;
    }

    print_repeat('^', res.len - start.col);
    printf("\n| ");
    if ( end.line - start.line > 1 ) printf("[...]\n| ");
    print_line(end.line, end.text, end.text_len);
    printf("| ");
    print_repeat('^', end.col);
    printf("\n");
    return;
}

void print_error(Nst_Error err)
{
    print_position(err.start, err.end);
    printf("%s - %s\n", err.name, err.message);
}

void print_traceback(Nst_Traceback tb)
{
    while ( tb.positions->head != NULL )
    {
        Pos start = *(Pos *)LList_pop(tb.positions);
        Pos end   = *(Pos *)LList_pop(tb.positions);
        print_position(start, end);
    }

    print_error(*tb.error);
}