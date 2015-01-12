#ifndef PHP_SASS_FUNCTIONS
#define PHP_SASS_FUNCTIONS

#include "sass.h"
#include <math.h>

union Sass_Value* call_fn_php(const union Sass_Value* psv_args, void* cookie);
union Sass_Value* call_fn_str_get(const union Sass_Value* psv_args, void* cookie);
union Sass_Value* call_fn_pow(const union Sass_Value* psv_args, void* cookie);
union Sass_Value* call_fn_gettype(const union Sass_Value* psv_args, void* cookie);
union Sass_Value* call_fn_remove_nth(const union Sass_Value* psv_args, void* cookie);

#endif
