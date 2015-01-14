#include "sass_functions.h"

union Sass_Value* call_fn_strip_unit(const union Sass_Value* psv_args, void* cookie) {
	if(sass_value_is_list(psv_args) && sass_list_get_length(psv_args) == 1) {
		union Sass_Value* psv_n = sass_list_get_value(psv_args, 0);
		if(sass_value_is_number(psv_n)) {
			return sass_make_number(sass_number_get_value(psv_n), "");
		}
		return sass_report_error("Argument in strip-unit must be number!");
	}
	return sass_report_error("Must have at least 1 variables in strip unit function call!");
}


union Sass_Value* call_fn_list_set(const union Sass_Value* psv_args, void* cookie) {
	if(sass_value_is_list(psv_args) && sass_list_get_length(psv_args) == 3) {
		union Sass_Value* psv_list = sass_list_get_value(psv_args, 0);
		if(sass_value_is_list(psv_list)) {
			union Sass_Value* psv_index = sass_list_get_value(psv_args, 1);
			if(sass_value_is_number(psv_index)) {
				int l = sass_list_get_length(psv_list);
				union Sass_Value* psv_ret = sass_make_list(l, sass_list_get_separator(psv_list));
				int i = 0;
				for(i = 0; i < l; i++) {
					if(i == sass_number_get_value(psv_index))
						sass_list_set_value(psv_ret, i, sass_dup_value(sass_list_get_value(psv_args, 2)));
					else
						sass_list_set_value(psv_ret, i, sass_dup_value(sass_list_get_value(psv_list, i)));
				}
				return psv_ret;
			}
			return sass_report_error("Argument 2 in list-set must be number!");
		}
		return sass_report_error("Argument 1 in list-set must be list!");
	}
	return sass_report_error("Must have at least 3 variables in list-set function call!");
}

union Sass_Value* call_fn_list_splice(const union Sass_Value* psv_args, void* cookie) {
	if(sass_value_is_list(psv_args) && sass_list_get_length(psv_args) == 4) {
		union Sass_Value* psv_list = sass_list_get_value(psv_args, 0);
		if(sass_value_is_list(psv_list)) {
			union Sass_Value* psv_offset = sass_list_get_value(psv_args, 1);
			if(sass_value_is_number(psv_offset)) {
				union Sass_Value* psv_count = sass_list_get_value(psv_args, 2);
				int l = sass_list_get_length(psv_list);
				int c = sass_number_get_value(psv_count);
				int offset = sass_number_get_value(psv_offset);

				int left = l - c;
				if(left < 0) {
					char buf[1024];
					sprintf(buf, "The first list's length %d is not long enough than the count %d!", l, c);
					return sass_report_error(buf);
				}

				if(offset < 0)
					offset += l;

				if(offset < 0)
					return sass_report_error("The first's length is not long enough than the minus offset!");

				union Sass_Value* psv_ret = NULL;
				int i;
				int j = 0;

				if(sass_value_is_number(psv_count)) {
					union Sass_Value* psv_list_append = sass_list_get_value(psv_args, 3);
					if(sass_value_is_list(psv_list_append)) {
						int l2 = sass_list_get_length(psv_list_append);

						psv_ret = sass_make_list(left + l2, sass_list_get_separator(psv_list));

						for(i = 0; i < l + l2; i++) {
							union Sass_Value* psv_v = NULL;
							if(i < offset) {
								psv_v = sass_list_get_value(psv_list, i);
							}
							else if(i >= offset && i < offset + c) {
								continue;
							}
							else if(i >= offset + c && i < l2 + offset + c) {
								psv_v = sass_list_get_value(psv_list_append, i - offset - c);
							}
							else {
								psv_v = sass_list_get_value(psv_list, i - l2);
							}
							sass_list_set_value(psv_ret, j++, sass_dup_value(psv_v));
						}
						return psv_ret;
					}
					else {
						if(sass_value_is_null(psv_list_append)) {
							psv_ret = sass_make_list(left, sass_list_get_separator(psv_list));
							for(i = 0; i < l; i++) {
								union Sass_Value* psv_v = NULL;
								if(i < offset) {
									psv_v = sass_list_get_value(psv_list, i);
								}
								else if(i >= offset && i < offset + c) {
									continue;
								}
								else {
									psv_v = sass_list_get_value(psv_list, i);
								}
								sass_list_set_value(psv_ret, j++, sass_dup_value(psv_v));
							}
							return psv_ret;
						}
						else {
							psv_ret = sass_make_list(left + 1, sass_list_get_separator(psv_list));
							for(i = 0; i < l + 1; i++) {
								union Sass_Value* psv_v = NULL;
								if(i < offset) {
									psv_v = sass_list_get_value(psv_list, i);
								}
								else if(i >= offset && i < offset + c) {
									continue;
								}
								else if(i == offset + c) {
									psv_v = psv_list_append;
								}
								else {
									psv_v = sass_list_get_value(psv_list, i - 1);
								}
								sass_list_set_value(psv_ret, j++, sass_dup_value(psv_v));
							}
							return psv_ret;
						}
					}
				}
				return sass_report_error("Argument 3 in list-splice must be number!");
			}
			return sass_report_error("Argument 2 in list-splice must be number!");
		}
		return sass_report_error("Argument 1 in list-splice must be list!");
	}
	return sass_report_error("Must have at least 4 variables in list-splice function call!");
}

union Sass_Value* call_fn_list_end(const union Sass_Value* psv_args, void* cookie) {
	if(sass_value_is_list(psv_args) && sass_list_get_length(psv_args) == 1) {
		union Sass_Value* psv_list = sass_list_get_value(psv_args, 0);
		if(sass_value_is_list(psv_list)) {
			int l = sass_list_get_length(psv_list);
			if(l) {
				return sass_dup_value(sass_list_get_value(psv_list, l - 1));
			}
			return sass_report_error("List is empty!");
		}
		return sass_report_error("Argument 1 in list-end must be list!");
		
	}
	return sass_report_error("Must have only 1 variables in list-end function call!");
}

union Sass_Value* sass_dup_value(union Sass_Value* psv_v) {
	if(sass_value_is_null(psv_v)) {
		return sass_make_null();
	}
	else if (sass_value_is_number(psv_v)) {
		return sass_make_number(sass_number_get_value(psv_v), sass_number_get_unit(psv_v));
	}
	else if (sass_value_is_string(psv_v)) {
		return sass_make_string(sass_string_get_value(psv_v));
	}
	else if (sass_value_is_boolean(psv_v)) {
		return sass_make_boolean(sass_boolean_get_value(psv_v));
	}
	else if (sass_value_is_color(psv_v)) {
		return sass_make_color(sass_color_get_r(psv_v),
			sass_color_get_g(psv_v),
			sass_color_get_b(psv_v),
			sass_color_get_a(psv_v));
	}
	else if (sass_value_is_list(psv_v)) {
		int l = sass_list_get_length(psv_v);
		union Sass_Value* psv_ret = sass_make_list(l, sass_list_get_separator(psv_v));
		int i = 0;
		for(i = 0; i < l; i++) {
			sass_list_set_value(psv_ret, i, sass_dup_value(sass_list_get_value(psv_v, i)));
		}
		return psv_ret;
	}
	else if (sass_value_is_map(psv_v)) {
		int l = sass_map_get_length(psv_v);
		union Sass_Value* psv_ret = sass_make_map(l);
		int i = 0;
		for(i = 0; i < l; i++) {
			sass_map_set_key(psv_ret, i, sass_dup_value(sass_map_get_key(psv_v, i)));
			sass_map_set_value(psv_ret, i, sass_dup_value(sass_map_get_value(psv_v, i)));
		}
		return psv_ret;
	}
	return sass_make_null();
}

union Sass_Value* call_fn_str_get(const union Sass_Value* psv_args, void* cookie) {
	if(sass_value_is_list(psv_args) && sass_list_get_length(psv_args) == 2) {
		union Sass_Value* psv_str = sass_list_get_value(psv_args, 0);
		union Sass_Value* psv_index = sass_list_get_value(psv_args, 1);
		if(sass_value_is_number(psv_index)) {
			int index = sass_number_get_value(psv_index);
			if(sass_value_is_string(psv_str)) {
				const char* s_str = sass_string_get_value(psv_str);
				if(index > 0 && index < strlen(s_str)) {
					char buf[] = {s_str[index-1], 0};
					return sass_make_string(buf);
				}
			}
			else {
				return sass_report_error("Argument 1 in str-get must be string!");
			}
		}
		else {
			return sass_report_error("Argument 2 in str-get must be number!");
		}
	}
	return sass_report_error("Must have 2 variables in str-get function call!");
}

union Sass_Value* call_fn_php(const union Sass_Value* psv_args, void* cookie) {
	if(sass_value_is_list(psv_args)) {
		const union Sass_Value* psv_params = psv_args;
		union Sass_Value* psv_func = sass_list_get_value(psv_args, 0);

		if(sass_value_is_list(psv_func)) {
			psv_params = psv_func;
			psv_func = sass_list_get_value(psv_func, 0);
			if(sass_value_is_string(psv_func)) {
				union Sass_Value* v = sass_php_call(sass_string_get_value(psv_func), psv_params);
				if(v)
					return v;
			}
			return sass_report_error("The first argument must be function name!");
		}
	}
	return sass_report_error("Call php failed!");
}

union Sass_Value* call_fn_pow(const union Sass_Value* psv_args, void* cookie) {
	if(sass_value_is_list(psv_args) && sass_list_get_length(psv_args) == 2) {
		union Sass_Value* psv_i = sass_list_get_value(psv_args, 0);
		union Sass_Value* psv_n = sass_list_get_value(psv_args, 1);
		if(sass_value_is_number(psv_i)) {
			if(sass_value_is_number(psv_n)) {
				double i = sass_number_get_value(psv_i);
				double n = sass_number_get_value(psv_n);
				return sass_make_number(pow(i, n), sass_number_get_unit(psv_i));
			}
			return sass_report_error("Argument 2 in pow must be number!");
		}
		return sass_report_error("Argument 1 in pow must be number!");
	}
	return sass_report_error("Must have 2 variables in pow function call!");
}

union Sass_Value* call_fn_remove_nth(const union Sass_Value* psv_args, void* cookie) {
	if(sass_value_is_list(psv_args) && sass_list_get_length(psv_args) == 2) {
		const union Sass_Value* psv_list = sass_list_get_value(psv_args, 0);
		const union Sass_Value* psv_n = sass_list_get_value(psv_args, 1);
		union Sass_Value* psv_ret = NULL;

		if(sass_value_is_list(psv_list)) {
			if(sass_value_is_number(psv_n)) {
				int n = sass_number_get_value(psv_n);
				int l = sass_list_get_length(psv_list);
				if(n > 0 && n <= l) {
					psv_ret = sass_make_list(l - 1, sass_list_get_separator(psv_list));
					int i = 0;
					int j = 0;
					for(i = 0; i < l; i++) {
						if(i == n - 1)
							continue;
						sass_list_set_value(psv_ret, j++, sass_dup_value(sass_list_get_value(psv_list, i)));
					}
					return psv_ret;
				}
				else {
					return sass_report_error("N must bigger than 0 and smaller than or equals to list's length");
				}
			}
			else {
				return sass_report_error("Argument 2 in remove-nth must be number!");
			}
		}
		else {
			return sass_report_error("Argument 1 in remove-nth must be list!");
		}
	}
	return sass_report_error("Must have 2 variables in remove-nth function call!");
}

union Sass_Value* call_fn_gettype(const union Sass_Value* psv_args, void* cookie) {
	if(sass_value_is_list(psv_args) && sass_list_get_length(psv_args) == 1) {
		const union Sass_Value* psv_arg = sass_list_get_value(psv_args, 0);
		if(sass_value_is_null(psv_arg)) {
			return sass_make_string("null");
		}
		else if (sass_value_is_number(psv_arg)) {
			return sass_make_string("number");
		}
		else if (sass_value_is_string(psv_arg)) {
			return sass_make_string("string");
		}
		else if (sass_value_is_boolean(psv_arg)) {
			return sass_make_string("boolean");
		}
		else if (sass_value_is_color(psv_arg)) {
			return sass_make_string("color");
		}
		else if (sass_value_is_list(psv_arg)) {
			return sass_make_string("list");
		}
		else if (sass_value_is_map(psv_arg)) {
			return sass_make_string("map");
		}
	}
	return sass_report_error("Must have only 1 variable in gettype function call!");
}
