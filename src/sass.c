#include "sass_options.h"

/**
 * Set all the options in the php hashtable and set them into Sass options.
 *
 * @args
 * 		sass_options: The options struct
 * 		php_options: The php hashtable
 */
void sass_set_options(struct Sass_Options* pso_options, zval* pzv_options) {
	HashTable* ht = Z_ARRVAL_P(pzv_options);
    HashPosition position;
    zval **data = NULL;

    bool found = false;
    // Iterating all the key and values in the context
    for (zend_hash_internal_pointer_reset_ex(ht, &position);
         zend_hash_get_current_data_ex(ht, (void**) &data, &position) == SUCCESS;
         zend_hash_move_forward_ex(ht, &position)) {

         char *key = NULL;
         uint  klen;
         ulong index;

         if (zend_hash_get_current_key_ex(ht, &key, &klen, &index, 0, &position) == HASH_KEY_IS_STRING) {
			 sass_set_option(pso_options, key, *data);
         }
    }
}

/**
 * Compile the input.
 *
 * @args
 * 		input: The input, for file compile, it is the file name, for string compile, it is the string
 * 		type: The type of the compile, support file or data
 * 		options: The php hashtable for all the options
 * 		error: The error string
 */
const char* sass_compile_context(char* s_input, const char* s_type, zval* pzv_options, zval* psv_error) {

	const char* ret = NULL;

	if(strcmp(s_type, SASS_TYPE_FILE) == 0) {
		// Initialize the context
		struct Sass_File_Context* psfc_file_ctx = sass_make_file_context(s_input);
		struct Sass_Context* psc_ctx = sass_file_context_get_context(psfc_file_ctx);
		struct Sass_Options* pso_ctx_opt = sass_context_get_options(psc_ctx);

		// Set the options from php
		sass_set_options(pso_ctx_opt, pzv_options);
		
		// Set it back to sass
		sass_file_context_set_options(psfc_file_ctx, pso_ctx_opt);

		// Do the compile
		int status = sass_compile_file_context(psfc_file_ctx);

		if(status) {
			// Set the error message
			ZVAL_STRING(psv_error, sass_context_get_error_message(psc_ctx), true);
		}
		else {
			ret = sass_context_get_output_string(psc_ctx);
		}

		// Clean the context
		sass_delete_file_context(psfc_file_ctx);

	}
	else if(strcmp(s_type, SASS_TYPE_DATA) == 0) {
		struct Sass_Data_Context* psdc_data_ctx = sass_make_data_context(s_input);
		struct Sass_Context* psc_ctx = sass_data_context_get_context(psdc_data_ctx);
		struct Sass_Options* pso_ctx_opt = sass_context_get_options(psc_ctx);

		sass_set_options(pso_ctx_opt, pzv_options);

		// Set it back to sass
		sass_data_context_set_options(psdc_data_ctx, pso_ctx_opt);

		// Do the compile
		int status = sass_compile_data_context(psdc_data_ctx);
		
		if(status) {
			// Set the error message
			ZVAL_STRING(psv_error, sass_context_get_error_message(psc_ctx), true);
		}
		else {
			ret = sass_context_get_output_string(psc_ctx);
		}
		// Clean the context
		sass_delete_data_context(psdc_data_ctx);
	}

	if(ret)
		return ret;

	return NULL;
}

void sass_set_option(struct Sass_Options* pso_options, const char* s_name, zval* pzv_option) {
	if(strcmp(s_name, SASS_OPTION_PRECISION) == 0) {
		sass_option_set_precision(pso_options, Z_LVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_OUTPUT_STYLE) == 0) {
		sass_option_set_output_style(pso_options, Z_LVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_COMMENTS) == 0) {
		sass_option_set_source_comments(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_MAP_EMBED) == 0) {
		sass_option_set_source_map_embed(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_MAP_CONTENTS) == 0) {
		sass_option_set_source_map_contents(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_OMIT_SOURCE_MAP_URL) == 0) {
		sass_option_set_omit_source_map_url(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_IS_INDENTED_SYNTAX_SRC) == 0) {
		sass_option_set_is_indented_syntax_src(pso_options, Z_BVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_INDENT) == 0) {
		sass_option_set_indent(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_LINEFEED) == 0) {
		sass_option_set_linefeed(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_INPUT_PATH) == 0) {
		sass_option_set_input_path(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_OUTPUT_PATH) == 0) {
		sass_option_set_output_path(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_IMAGE_PATH) == 0) {
		sass_option_set_image_path(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_INCLUDE_PATH) == 0) {
		sass_option_set_include_path(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_MAP_FILE) == 0) {
		sass_option_set_source_map_file(pso_options, Z_STRVAL_P(pzv_option));
	}
}

/*******************************************************************************
 *
 *  Function sass_compile
 *
 *  This function will run the sass compiler using the options
 *
 *  @version 1.0
 *  @args
 *
 *******************************************************************************/
PHP_FUNCTION(sass_compile) {
	char* s_type = NULL;
	int type_len;
	char* s_input = NULL;
	int input_len = 0;
	zval* pzv_options = NULL;
	zval* pzv_error = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssAz", &s_type, &type_len,
			   	&s_input, &input_len,
				&pzv_options, &pzv_error) == SUCCESS) {
		const char* s_ret = sass_compile_context(s_input, s_type, pzv_options, pzv_error);

		if(s_ret)
			RETURN_STRING(s_ret, true);

		RETURN_FALSE;
	}
	RETURN_FALSE;
}


/*******************************************************************************
 *
 *  Function sass_version
 *
 *  This function will return the version of the libsass
 *
 *  @version 1.0
 *
 *******************************************************************************/
PHP_FUNCTION(sass_version) {
	RETURN_STRING(libsass_version(), true);
}

static zend_function_entry sass_functions[] = {
    PHP_FE(sass_compile, NULL)   
    PHP_FE(sass_version, NULL)   
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

