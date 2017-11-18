#include "sass_options.h"
#include "sass_functions.h"
#include <json.hpp>
#include <string.h>
#include <sass.hpp>
#include <context.hpp>

using namespace std;
using namespace Sass;

// sass config options structure
struct Sass_Options_Fake {
	int precision;
	enum Sass_Output_Style output_style;
	bool source_comments;
	bool source_map_embed;
	bool source_map_contents;
	bool omit_source_map_url;
	bool is_indented_syntax_src;
	char* input_path;
	char* output_path;
	const char* indent;
	const char* linefeed;
	char* include_path;
	char* plugin_path;
	struct string_list* include_paths;
	struct string_list* plugin_paths;
	char* source_map_file;
	char* source_map_root;
	Sass_Function_List c_functions;
	Sass_Importer_List c_importers;
	Sass_Importer_List c_headers;
};

// base for all contexts
struct Sass_Context_Fake : Sass_Options_Fake {
	int type;
	char* output_string;
	char* source_map_string;
	int error_status;
	char* error_json;
	char* error_text;
	char* error_message;
	char* error_file;
	size_t error_line;
	size_t error_column;
	const char* error_src;
	char** included_files;
};

struct Sass_Compiler_Fake {
	Sass_Compiler_State state;
	Sass_Context* c_ctx;
	Context* cpp_ctx;
	Block* root;
};


/**
 * Checking the arguments according to the argument check string. The types and the names of the string is:
 *
 * - *: variable arguments
 * - s: string
 * - 0: null
 * - n: number
 * - l: list
 * - m: map
 * - c: color
 * - b: boolean
 * - ?: any type
 */
bool sass_check_args(const char* s_args, int count, const union Sass_Value* psv_args_list) {
	if(sass_value_is_list(psv_args_list) && sass_list_get_length(psv_args_list) == count) {
		int i;
		for(i = 0; i < count; i++) {
			char c = s_args[i];
			union Sass_Value* psv_v = sass_list_get_value(psv_args_list, i);
			switch(c) {
				case 's':
					if(!sass_value_is_string(psv_v))
						return false;
					break;
				case '0':
					if(!sass_value_is_null(psv_v))
						return false;
					break;
				case 'n':
					if(!sass_value_is_number(psv_v))
						return false;
					break;
				case '*':
					if(!(sass_value_is_list(psv_v) && i == 0)) // For variable args, the argument list must only have 1 argument(as list)
						return false;
					break;
				case 'l':
					if(!sass_value_is_list(psv_v)) // For variable args, the argument list must only have 1 argument(as list)
						return false;
					break;
				case 'm':
					if(!sass_value_is_map(psv_v))
						return false;
					break;
				case 'b':
					if(!sass_value_is_boolean(psv_v))
						return false;
					break;
			}
		}
		return true;
	}
	return false;
}

int sass_php_count(HashTable* ht) {
	HashPosition position;

	int count = 0;
	// Iterating all the key and values in the context
	for (zend_hash_internal_pointer_reset_ex(ht, &position);
		 zend_hash_get_current_data_ex(ht, &position) != NULL;
		 zend_hash_move_forward_ex(ht, &position)) {
		 count++;
	}
	return count;
}

union Sass_Value* sass_report_error(const char* s_error) {
	return sass_make_error(s_error);
}

void sass_to_php(union Sass_Value* psv_arg, zval* pzv_arg) {
	if(sass_value_is_null(psv_arg)) {
		ZVAL_NULL(pzv_arg);
	}
	else if (sass_value_is_number(psv_arg)) {
		ZVAL_DOUBLE(pzv_arg, sass_number_get_value(psv_arg));
	}
	else if (sass_value_is_string(psv_arg)) {
		ZVAL_STRING(pzv_arg, sass_string_get_value(psv_arg));
	}
	else if (sass_value_is_boolean(psv_arg)) {
		if(sass_boolean_get_value(psv_arg)) {
			ZVAL_TRUE(pzv_arg);
		}
		else {
			ZVAL_FALSE(pzv_arg);
		}
	}
	else if (sass_value_is_color(psv_arg)) {
		char color[24];
		snprintf(color, sizeof color, "#%.2x%.2x%.2x", 
			(int)sass_color_get_r(psv_arg), 
			(int)sass_color_get_g(psv_arg), 
			(int)sass_color_get_b(psv_arg));
		ZVAL_STRING(pzv_arg, color);
	}
	else if (sass_value_is_list(psv_arg)) {

		size_t count = sass_list_get_length(psv_arg);

		// Initlialize the php arg as an array
		array_init_size(pzv_arg, count);

		int i;
		
		for(i = 0; i < count; i++) {
			zval zv_array_item;

			// Conver the list item to php
			sass_to_php(sass_list_get_value(psv_arg, i), &zv_array_item);

			// Add the array item to the array
			add_next_index_zval(pzv_arg, &zv_array_item);
		}
	}
	else if (sass_value_is_map(psv_arg)) {
		size_t count = sass_map_get_length(psv_arg);

		// Initlialize the php arg as an array
		array_init_size(pzv_arg, count);

		int i;
		
		for(i = 0; i < count; i++) {
			zval zv_array_item;

			// Conver the list item to php
			sass_to_php(sass_map_get_value(psv_arg, i), &zv_array_item);

			// Add the array item to the array
			add_assoc_zval(pzv_arg, sass_string_get_value(sass_map_get_key(psv_arg, i)), &zv_array_item);
		}
	}
}

union Sass_Value* convert_php_to_list(zval* pzv_val) {
	int count = sass_php_count(Z_ARRVAL_P(pzv_val));
	union Sass_Value** values = static_cast<union Sass_Value**>(safe_emalloc(sizeof(union Sass_Value*), count, 0));

	HashTable* ht = Z_ARRVAL_P(pzv_val);
	HashPosition position;
	zval* pzv_data = NULL;

	int real_count = 0;
	// Iterating all the key and values in the context
	for (zend_hash_internal_pointer_reset_ex(ht, &position);
		 (pzv_data = zend_hash_get_current_data_ex(ht, &position)) != NULL;
		 zend_hash_move_forward_ex(ht, &position)) {

		 zend_string *key = NULL;
		 zend_ulong index;

		 // Only require the string key, all the index key will be ignored
		 if (zend_hash_get_current_key_ex(ht, &key, &index, &position) == HASH_KEY_IS_LONG) {
			 values[real_count] = php_to_sass(pzv_data);
			 real_count++;
		 }
	}

	// Create the map
	union Sass_Value* psv_list = sass_make_list(real_count, SASS_SPACE, true);

	int i;
	for(i = 0; i < real_count; i++) {
		sass_list_set_value(psv_list, i, values[i]);
	}

	efree(values);

	return psv_list;
}

union Sass_Value* convert_php_to_map(zval* pzv_val) {
	int count = sass_php_count(Z_OBJPROP_P(pzv_val));
	zend_string** keys = static_cast<zend_string**>(emalloc((count + 1) * sizeof(zend_string*)));
	union Sass_Value** values = static_cast<union Sass_Value**>(emalloc((count + 1) * sizeof(union Sass_Value*)));

	HashTable* ht = Z_OBJPROP_P(pzv_val);
	HashPosition position;
	zval* pzv_data = NULL;

	int real_count = 0;
	// Iterating all the key and values in the context
	for (zend_hash_internal_pointer_reset_ex(ht, &position);
		 (pzv_data = zend_hash_get_current_data_ex(ht, &position)) != NULL;
		 zend_hash_move_forward_ex(ht, &position)) {

		 zend_string *key = NULL;
		 zend_ulong index;

		 // Only require the string key, all the index key will be ignored
		 if (zend_hash_get_current_key_ex(ht, &key, &index, &position) == HASH_KEY_IS_STRING) {
			 keys[real_count] = key;
			 values[real_count] = php_to_sass(pzv_data);
			 real_count++;
		 }
	}

	// Create the map
	union Sass_Value* psv_map = sass_make_map(real_count);

	int i;
	for(i = 0; i < real_count; i++) {
		sass_map_set_key(psv_map, i, sass_make_string(keys[i]->val));
		sass_map_set_value(psv_map, i, values[i]);
	}

	efree(keys);
	efree(values);

	return psv_map;
}

union Sass_Value* php_to_sass(zval* pzv_arg) {
	switch(Z_TYPE_P(pzv_arg)) {
	case IS_LONG:
		return sass_make_number(Z_LVAL_P(pzv_arg), "");
	case IS_DOUBLE:
		return sass_make_number(Z_DVAL_P(pzv_arg), "");
	case IS_TRUE:
		return sass_make_boolean(true);
	case IS_FALSE:
		return sass_make_boolean(false);
	case IS_ARRAY:
		// Will treat array as list and object as map
		return convert_php_to_list(pzv_arg);
	case IS_OBJECT:
		return convert_php_to_map(pzv_arg);
	case IS_STRING:
		return sass_make_string(Z_STRVAL_P(pzv_arg));
	}
	return sass_make_null();
}

union Sass_Value* sass_php_call(const char* s_func, const union Sass_Value* psv_args) {
	size_t args_count = sass_list_get_length(psv_args) - 1; // Skip the first arg of function name

	// Copy the function name to php variable
    zval zv_function_name;
    ZVAL_STRING(&zv_function_name, s_func);

    size_t argc = sass_list_get_length(psv_args) - 1;

    // Initialize the paramter array
    zval* pzv_params = (zval*) emalloc(argc * sizeof(zval));
    zval zv_php_ret_val;

	int i;
	// Setup the input parameters
	for(i = 0; i < argc; i++) {
		// Initialize the php value
		zval zv_val;

		// Getting sass value
		union Sass_Value* psv_val = sass_list_get_value(psv_args, i + 1);
		
		// Convert it to php
		sass_to_php(psv_val, &zv_val);

		// Adding it to the php args
		pzv_params[i] = zv_val;
	}

	union Sass_Value* psv_ret = NULL;

    if (call_user_function(EG(function_table), NULL, &zv_function_name, &zv_php_ret_val, argc, pzv_params TSRMLS_CC) == SUCCESS) {
		psv_ret = php_to_sass(&zv_php_ret_val);
	}

    // Destroy all the php parameter variables
    for(i = 0; i < argc; i++) {
		zval_dtor(&pzv_params[i]);
    }

	// Free the args list
    efree(pzv_params);

    // Destroy the php return variable
    zval_dtor(&zv_php_ret_val);

    // Destroy the php function name variable
    zval_dtor(&zv_function_name);

	if(psv_ret)
		return psv_ret;

	char buff[256];
	sprintf(buff, "Failed to call php function %s!", s_func);
	return sass_report_error(buff);
}

/**
 * Set all the options in the php hashtable and set them into Sass options.
 *
 * @args
 * 		sass_options: The options struct
 * 		php_options: The php hashtable
 */
void sass_set_options(struct Sass_Options* pso_options, zval* pzv_options) {
	if(pzv_options) {
		HashTable* ht = Z_ARRVAL_P(pzv_options);
		HashPosition position;
		zval* pzv_data = NULL;

		// Iterating all the key and values in the context
		for (zend_hash_internal_pointer_reset_ex(ht, &position);
			 (pzv_data = zend_hash_get_current_data_ex(ht, &position)) != NULL;
			 zend_hash_move_forward_ex(ht, &position)) {

			 zend_string *key = NULL;
			 zend_ulong index;

			 if (zend_hash_get_current_key_ex(ht, &key, &index, &position) == HASH_KEY_IS_STRING) {
				 sass_set_option(pso_options, key->val, pzv_data);
			 }
		}
	}


	// create list of all custom functions
	int i = 0;
	Sass_Function_List fn_list = sass_make_function_list(13);
	SASS_FUNCTION(call_fn_php, "php($func...)");
	SASS_FUNCTION(call_fn_str_get, "str-get($str, $index)");
	SASS_FUNCTION(call_fn_pow, "pow($i, $n)");
	SASS_FUNCTION(call_fn_gettype, "gettype($i)");
	SASS_FUNCTION(call_fn_remove_nth, "remove-nth($l, $i)");
	SASS_FUNCTION(call_fn_list_start, "first($l)");
	SASS_FUNCTION(call_fn_list_end,"last($l)");
	SASS_FUNCTION(call_fn_list_reverse,"reverse($l)");
	SASS_FUNCTION(call_fn_list_splice,"list-splice($list, $offset:0, $count:0, $list_append:null)");
	SASS_FUNCTION(call_fn_list_set,"list-set($list, $offset, $value)");
	SASS_FUNCTION(call_fn_strip_unit,"strip-unit($n)");
	SASS_FUNCTION(call_fn_assert,"assert($b, $m)");
	SASS_FUNCTION(call_fn_convert_unit,"convert-unit($n, $u)");
	sass_option_set_c_functions(pso_options, fn_list);
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
bool sass_compile_context(char* s_input, const char* s_type, zval* pzv_options, zval* pzv_ret, zval* psv_error) {

	bool is_file = strcmp(s_type, SASS_TYPE_FILE) == 0;
	struct Sass_Context* psc_ctx = NULL;
	struct Sass_Options* pso_ctx_opt = NULL;

	if(is_file) {
		psc_ctx = sass_file_context_get_context(sass_make_file_context(s_input));
	}
	else {
		psc_ctx = sass_data_context_get_context(sass_make_data_context(s_input));
	}

	pso_ctx_opt = sass_context_get_options(psc_ctx);

	// Set the options from php
	sass_set_options(pso_ctx_opt, pzv_options);
	
	// Set it back to sass
	if(is_file) {
		sass_file_context_set_options((struct Sass_File_Context*)(psc_ctx), pso_ctx_opt);
	}
	else {
		sass_data_context_set_options((struct Sass_Data_Context*)(psc_ctx), pso_ctx_opt);
	}

	struct Sass_Compiler_Fake* fake = NULL;

	if(is_file) {
		fake = (struct Sass_Compiler_Fake *) sass_make_file_compiler((struct Sass_File_Context*)(psc_ctx));
	}
	else {
		fake = (struct Sass_Compiler_Fake *) sass_make_data_compiler((struct Sass_Data_Context*)(psc_ctx));
	}

	Context* context = fake->cpp_ctx;


	struct Sass_Compiler* compiler = (struct Sass_Compiler*) fake;

	// call each compiler step
	sass_compiler_parse(compiler);
	sass_compiler_execute(compiler);
    sass_delete_compiler(compiler);

	int status = ((struct Sass_Context_Fake *) psc_ctx)->error_status;

	if(status) {
		// Set the error message
		add_next_index_string(psv_error, sass_context_get_error_message(psc_ctx));
	}
	else {
		ZVAL_STRING(pzv_ret, sass_context_get_output_string(psc_ctx));
	}

	if(is_file) {
		// Clean the context
		sass_delete_file_context((struct Sass_File_Context*)(psc_ctx));
	}
	else {
		sass_delete_data_context((struct Sass_Data_Context*)(psc_ctx));
	}

	if(pzv_ret)
		return true;

	return false;
}

bool _zval_to_bool(zval* pzv_value) {
    if(Z_TYPE_P(pzv_value) == IS_TRUE) {
        return true;
    }
    return false;
}

void sass_set_option(struct Sass_Options* pso_options, const char* s_name, zval* pzv_option) {
	if(strcmp(s_name, SASS_OPTION_PRECISION) == 0) {
		sass_option_set_precision(pso_options, Z_LVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_OUTPUT_STYLE) == 0) {
		sass_option_set_output_style(pso_options, static_cast<enum Sass_Output_Style>(Z_LVAL_P(pzv_option)));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_COMMENTS) == 0) {
        sass_option_set_source_comments(pso_options, _zval_to_bool(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_MAP_EMBED) == 0) {
		sass_option_set_source_map_embed(pso_options, _zval_to_bool(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_MAP_CONTENTS) == 0) {
		sass_option_set_source_map_contents(pso_options, _zval_to_bool(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_OMIT_SOURCE_MAP_URL) == 0) {
		sass_option_set_omit_source_map_url(pso_options, _zval_to_bool(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_SOURCE_MAP_FILE) == 0) {
		sass_option_set_source_map_file(pso_options, Z_STRVAL_P(pzv_option));
	}
	else if(strcmp(s_name, SASS_OPTION_IS_INDENTED_SYNTAX_SRC) == 0) {
		sass_option_set_is_indented_syntax_src(pso_options, _zval_to_bool(pzv_option));
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
	char* s_type;
	int type_len = 0;
	char* s_input;
	int input_len = 0;
	zval* pzv_options;
	zval* pzv_error;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssAa", &s_type, &type_len,
			   	&s_input, &input_len,
				&pzv_options, &pzv_error) == SUCCESS) {
		s_input = strdup(s_input);
		if(sass_compile_context(s_input, s_type, pzv_options, return_value, pzv_error)) {
			return;
		}

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
	RETURN_STRING(libsass_version());
}

PHP_FUNCTION(sass_is_complete) {
	char* s_input = NULL;
	int input_len = 0;
	if(zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_input, &input_len) == SUCCESS) {
		struct Sass_Data_Context* psdc_data_ctx = sass_make_data_context(s_input);
		struct Sass_Context* psc_ctx = sass_data_context_get_context(psdc_data_ctx);
		struct Sass_Options* pso_ctx_opt = sass_context_get_options(psc_ctx);

		sass_set_options(pso_ctx_opt, NULL);

		sass_data_context_set_options(psdc_data_ctx, pso_ctx_opt);

		// Do the compile
		int status = sass_compile_data_context(psdc_data_ctx);
		
		// Clean the context
		sass_delete_data_context(psdc_data_ctx);

		if(status) {
			RETURN_FALSE;
		}
		else {
			RETURN_TRUE;
		}
	}
	RETURN_FALSE;
}

static PHP_MINFO_FUNCTION(sass) {
    php_info_print_table_start();
    php_info_print_table_row(2, "Revision", PHP_SASS_VERSION);
    php_info_print_table_row(2, "LibSass Revision", libsass_version());
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();    
}

static zend_function_entry sass_functions[] = {
    PHP_FE(sass_compile, NULL)   
    PHP_FE(sass_version, NULL)   
    PHP_FE(sass_is_complete, NULL)   
	PHP_FE_END
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
	PHP_MINFO(sass),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_SASS_VERSION,         
#endif
    STANDARD_MODULE_PROPERTIES 
};
  
#ifdef COMPILE_DL_SASS
ZEND_GET_MODULE(sass)
#endif

