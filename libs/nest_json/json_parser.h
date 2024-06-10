#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "nest.h"

extern bool trailing_commas;
const bool trailing_commas_default = false;

Nst_Obj *json_parse(i8 *path, Nst_LList *tokens);

#endif // !JSON_PARSER