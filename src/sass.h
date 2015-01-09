#ifndef PHP_SASS
#define PHP_SASS

#ifdef HAVE_CONFIG_H           
#include "config.h"            
#endif
    
#define PHP_SASS_VERSION "0.0.1"
#define PHP_SASS_EXTNAME "sass"

#include <php.h>
#include <libsass/sass_context.h>

PHP_FUNCTION(sass_compile); 

#define phpext_clips_ptr &sass_module_entry                                                             

#endif
