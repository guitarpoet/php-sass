#include "sass.h"

PHP_FUNCTION(sass_compile) {
	printf("Faking compile");
}

static zend_function_entry sass_functions[] = {
    PHP_FE(sass_compile, NULL)   
    {NULL, NULL, NULL}         
};
  
zend_module_entry sass_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,    
#endif
    PHP_SASS_EXTNAME,         
    sass_functions,           
    NULL, // No initialize functions
	NULL, // No deinitialize functions
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_SASS_VERSION,         
#endif
    STANDARD_MODULE_PROPERTIES 
};
  
#ifdef COMPILE_DL_SASS
ZEND_GET_MODULE(sass)
#endif

