#ifndef JSON_DUMPER_H
#define JSON_DUMPER_H

#include "nest.h"

Nst_Obj *json_dump(Nst_Obj *obj, i32 indent, Nst_OpErr *err);

#endif // !JSON_DUMPER_H