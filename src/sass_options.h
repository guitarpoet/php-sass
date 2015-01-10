#ifndef PHP_SASS_OPTIONS
#define PHP_SASS_OPTIONS

#include "sass.h"

#define SASS_OPTION_PRECISION "precision"
#define SASS_OPTION_OUTPUT_STYLE "output_style"
#define SASS_OPTION_SOURCE_COMMENTS "source_comments"
#define SASS_OPTION_SOURCE_MAP_EMBED "source_map_embed"
#define SASS_OPTION_SOURCE_MAP_CONTENTS "source_map_contents"
#define SASS_OPTION_OMIT_SOURCE_MAP_URL "omit_source_map_url"
#define SASS_OPTION_IS_INDENTED_SYNTAX_SRC "is_indented_syntax_src"
#define SASS_OPTION_INDENT "indent"
#define SASS_OPTION_LINEFEED "linefeed"
#define SASS_OPTION_INPUT_PATH "input_path"
#define SASS_OPTION_OUTPUT_PATH "output_path"
#define SASS_OPTION_IMAGE_PATH "image_path"
#define SASS_OPTION_INCLUDE_PATH "include_path"
#define SASS_OPTION_SOURCE_MAP_FILE "source_map_file"

void sass_set_option(struct Sass_Options* options, const char* name, zval* option);

#endif

