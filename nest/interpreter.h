#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "nodes.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

void run(Node *node);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H