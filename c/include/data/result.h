#ifndef RESULT_H
#define RESULT_H

#include <string.h>

#define DEFINE_RESULT(type) struct type##_result {bool is_ok; union {struct type *value; char *error;};};
#define RESULT(type) type##_result

#define make_ok(result, val)            \
        do {                            \
                result.value = val;     \
                result.is_ok = true;    \
        }                               \
        while (0)                       \

#define make_error(result, err) \
do {                            \
        result.error = err;     \
        result.is_ok = false;   \
}                               \
while (0)                       \

#define make_error_lit(result, lit)                                     \
do {                                                                    \
        char *err = (char*) malloc((strlen(lit) + 1) * sizeof(char));   \
        strcpy(err, lit);                                               \
        result.error = err;                                             \
        result.is_ok = false;                                           \
}                                                                       \
while (0)                                                               \

#endif
