/*
 * Copyright (C) 2013 Jesse Weaver <pianohacker@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <glib.h>
#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include <math.h>
#include <stdbool.h>

#include "types.h"

// Large files of this are liberally copied from glib's gvaluetypes.c.
// Many thanks to their original authors.

#define DEF_GET_SET(suffix, upper_suffix, dup_function, free_function, type) \
                                                                             \
void gsdl_gvalue_set_##suffix(GValue *value, const type *src) {              \
	gpointer new_val;                                                        \
                                                                             \
	g_return_if_fail(GSDL_GVALUE_HOLDS_##upper_suffix(value));               \
                                                                             \
	new_val = dup_function(src);                                             \
                                                                             \
	if (value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS) {                   \
		value->data[1].v_uint = 0;                                           \
	} else {                                                                 \
		free_function(value->data[0].v_pointer);                             \
	}                                                                        \
                                                                             \
	value->data[0].v_pointer = new_val;                                      \
}                                                                            \
                                                                             \
void gsdl_gvalue_take_##suffix(GValue *value, type *src) {                   \
	g_return_if_fail(GSDL_GVALUE_HOLDS_##upper_suffix(value));               \
                                                                             \
	if (value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS) {                   \
		value->data[1].v_uint = 0;                                           \
	} else {                                                                 \
		free_function(value->data[0].v_pointer);                             \
	}                                                                        \
                                                                             \
	value->data[0].v_pointer = src;                                          \
}                                                                            \
                                                                             \
const type* gsdl_gvalue_get_##suffix(const GValue *value) {                  \
	g_return_val_if_fail(GSDL_GVALUE_HOLDS_##upper_suffix(value), NULL);     \
                                                                             \
	return (type*) value->data[0].v_pointer;                                 \
}

#define DEF_POINTER_VALUE(suffix, upper_suffix, dup_function, free_function)                        \
GType GSDL_TYPE_##upper_suffix;                                                                     \
                                                                                                    \
static gchar* _value_collect_##suffix(                                                              \
		GValue *value,                                                                              \
		guint n_collect_values,                                                                     \
		GTypeCValue *collect_values,                                                                \
		guint collect_flags                                                                         \
	) {                                                                                             \
                                                                                                    \
	if (!collect_values[0].v_pointer) {                                                             \
		value->data[0].v_pointer = NULL;                                                            \
	} else if (collect_flags & G_VALUE_NOCOPY_CONTENTS) {                                           \
		value->data[0].v_pointer = collect_values[0].v_pointer;                                     \
		value->data[1].v_uint = G_VALUE_NOCOPY_CONTENTS;                                            \
	} else {                                                                                        \
		value->data[0].v_pointer = dup_function(collect_values[0].v_pointer);                       \
	}                                                                                               \
                                                                                                    \
	return NULL;                                                                                    \
}                                                                                                   \
                                                                                                    \
static void _value_free_##suffix(GValue *value) {                                                   \
	if (!(value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS)) {                                       \
		free_function(value->data[0].v_pointer);                                                    \
	}                                                                                               \
}                                                                                                   \
                                                                                                    \
static void _value_copy_##suffix(const GValue *src_value, GValue *dest_value) {                     \
	dest_value->data[0].v_pointer = dup_function(src_value->data[0].v_pointer);                     \
}                                                                                                   \
                                                                                                    \
static gchar* _value_lcopy_##suffix(                                                                \
		const GValue *value,                                                                        \
		guint n_collect_values,                                                                     \
		GTypeCValue *collect_values,                                                                \
		guint collect_flags                                                                         \
	) {                                                                                             \
                                                                                                    \
	gchar **string_p = collect_values[0].v_pointer;                                                 \
                                                                                                    \
	if (!string_p) {                                                                                \
		return g_strdup_printf("value location for `%s' passed as NULL", G_VALUE_TYPE_NAME(value)); \
	}                                                                                               \
                                                                                                    \
	if (!value->data[0].v_pointer) {                                                                \
		*string_p = NULL;                                                                           \
	} else if (collect_flags & G_VALUE_NOCOPY_CONTENTS) {                                           \
		*string_p = value->data[0].v_pointer;                                                       \
	} else {                                                                                        \
		*string_p = dup_function(value->data[0].v_pointer);                                         \
	}                                                                                               \
                                                                                                    \
	return NULL;                                                                                    \
}

#define REGISTER_POINTER_VALUE(suffix, upper_suffix)                                                                \
	static const GTypeValueTable suffix##_value_table = {                                                           \
		value_init: _value_init_pointer,                                                                            \
		value_free: _value_free_##suffix,                                                                           \
		value_copy: _value_copy_##suffix,                                                                           \
		value_peek_pointer: _value_peek_pointer,                                                                    \
		collect_format: "p",                                                                                        \
		collect_value: _value_collect_##suffix,                                                                     \
		lcopy_format: "p",                                                                                          \
		lcopy_value: _value_lcopy_##suffix,                                                                         \
	};                                                                                                              \
                                                                                                                    \
	info.value_table = &suffix##_value_table;                                                                           \
	GSDL_TYPE_##upper_suffix = g_type_fundamental_next();                                                           \
	g_type_register_fundamental(GSDL_TYPE_##upper_suffix, g_intern_static_string("gsdl"#suffix), &info, &finfo, 0); \
	g_value_register_transform_func(GSDL_TYPE_##upper_suffix, G_TYPE_STRING, _value_transform_##suffix##_string);       

static inline gpointer _g_byte_array_dup(gconstpointer src) {
	GByteArray *src_array = (GByteArray*) src;

	return g_byte_array_new_take(g_memdup(src_array->data, src_array->len), src_array->len);
}

static inline gpointer _g_date_dup(gconstpointer src) {
	return g_memdup(src, sizeof(GDate));
}

static inline gpointer _g_date_time_dup(gconstpointer src) {
	return g_date_time_add((GDateTime*) src, 0);
}

static inline void _g_byte_array_free(gconstpointer src) {
	if (src) g_byte_array_unref((GByteArray*) src);
}

static inline void _g_date_time_free(gconstpointer src) {
	if (src) g_date_time_unref((GDateTime*) src);
}

DEF_GET_SET(binary, BINARY, _g_byte_array_dup, _g_byte_array_free, GByteArray)
DEF_GET_SET(decimal, DECIMAL, g_strdup, g_free, gchar)
DEF_GET_SET(date, DATE, _g_date_dup, g_free, GDate)
DEF_GET_SET(datetime, DATETIME, _g_date_time_dup, _g_date_time_free, GDateTime)

void gsdl_gvalue_set_timespan(GValue *value, const GTimeSpan src) {
	g_return_if_fail(GSDL_GVALUE_HOLDS_TIMESPAN(value));

	value->data[0].v_int64 = src;
}

GTimeSpan gsdl_gvalue_get_timespan(const GValue *value) {
	g_return_val_if_fail(GSDL_GVALUE_HOLDS_TIMESPAN(value), 0);

	return value->data[0].v_int64;
}

void gsdl_gvalue_set_unichar(GValue *value, const gunichar src) {
	g_return_if_fail(GSDL_GVALUE_HOLDS_UNICHAR(value));

	value->data[0].v_int64 = src;
}

gunichar gsdl_gvalue_get_unichar(const GValue *value) {
	g_return_val_if_fail(GSDL_GVALUE_HOLDS_UNICHAR(value), 0);

	return value->data[0].v_int64;
}

GType GSDL_TYPE_TIMESPAN;
GType GSDL_TYPE_UNICHAR;
DEF_POINTER_VALUE(binary, BINARY, _g_byte_array_dup, _g_byte_array_free);
DEF_POINTER_VALUE(decimal, DECIMAL, g_strdup, g_free);
DEF_POINTER_VALUE(date, DATE, _g_date_dup, g_free);
DEF_POINTER_VALUE(datetime, DATETIME, _g_date_time_dup, _g_date_time_free);

static void _value_init_pointer(GValue *value) {
	value->data[0].v_pointer = NULL;
}

static gpointer _value_peek_pointer(const GValue *value) {
	return value->data[0].v_pointer;
}

static void _value_transform_binary_string(const GValue *src_value, GValue *dest_value) {
	GByteArray *src = (GByteArray*) src_value->data[0].v_pointer;
	char *result = g_malloc(src->len * 4 + 1);
	guchar *in = src->data;
	guchar *end = in + src->len;
	
	char *out = result;

	for (; in < end; in++) {
		if (*in == 0) {
			*out++ = '\\';
			*out++ = '0';
		} else if (*in < 32 || *in > 127) {
			*out++ = '\\';
			*out++ = 'x';

			int digit = *in >> 4;
			*out++ = digit < 10 ? '0' + digit : 'a' + digit - 10;
			digit = *in % 16;
			*out++ = digit < 10 ? '0' + digit : 'a' + digit - 10;
		} else {
			*out++ = (gchar) *in;
		}
	}

	*out++ = '\0';

	dest_value->data[0].v_pointer = result;;
}

static void _value_transform_decimal_string(const GValue *src_value, GValue *dest_value) {
	dest_value->data[0].v_pointer = g_strdup(src_value->data[0].v_pointer);
}

static void _value_transform_date_string(const GValue *src_value, GValue *dest_value) {
	GDate *src = (GDate*) src_value->data[0].v_pointer;
	dest_value->data[0].v_pointer = g_strdup_printf("%04d-%02d-%02d", src->year, src->month, src->day);
}

static void _value_transform_datetime_string(const GValue *src_value, GValue *dest_value) {
	GDateTime *src = (GDateTime*) src_value->data[0].v_pointer;
	gdouble trash;
	int milliseconds = round(modf(g_date_time_get_seconds(src), &trash) * 1000);

	if (milliseconds) {
		dest_value->data[0].v_pointer = g_strdup_printf("%s.%03d%s", g_date_time_format(src, "%FT%T"), milliseconds, g_date_time_format(src, "%z"));
	} else {
		dest_value->data[0].v_pointer = g_date_time_format(src, "%FT%T%z");
	}
}

static void _value_init_int64(GValue *value) {
}

static void _value_copy_int64(const GValue *src_value, GValue *dest_value) {
	dest_value->data[0].v_int64 = src_value->data[0].v_int64;
}

static gchar* _value_collect_int64(
		GValue *value,
		guint n_collect_values,
		GTypeCValue *collect_values,
		guint collect_flags
	) {

	value->data[0].v_int64 = collect_values[0].v_int64;

	return NULL;
}

static gchar* _value_lcopy_int64(
		const GValue *value,
		guint n_collect_values,
		GTypeCValue *collect_values,
		guint collect_flags
	) {

	gint64 *timespan_p = collect_values[0].v_pointer;

	if (!timespan_p) {
		return g_strdup_printf("value location for `%s' passed as NULL", G_VALUE_TYPE_NAME(value));
	}

	*timespan_p = value->data[0].v_int64;

	return NULL;
}

static void _value_transform_timespan_string(const GValue *src_value, GValue *dest_value) {
	dest_value->data[0].v_pointer = g_strdup_printf("%"G_GINT64_FORMAT, src_value->data[0].v_int64);
}

static void _value_transform_unichar_string(const GValue *src_value, GValue *dest_value) {
	char *value = g_malloc(7);
	value[g_unichar_to_utf8(src_value->data[0].v_int64, value)] = 0;

	dest_value->data[0].v_pointer = value;
}

void _gsdl_types_init() {
	static bool init_done = false;
	if (init_done) return;

	GTypeInfo info = {
		class_size: 0,
		base_init: NULL,
		class_init: NULL,
		class_data: NULL,
		instance_size: 0,
		n_preallocs: 0,
		instance_init: NULL,
		value_table: NULL,
	};
	const GTypeFundamentalInfo finfo = { G_TYPE_FLAG_DERIVABLE, };

	REGISTER_POINTER_VALUE(binary, BINARY);
	REGISTER_POINTER_VALUE(decimal, DECIMAL);
	REGISTER_POINTER_VALUE(date, DATE);
	REGISTER_POINTER_VALUE(datetime, DATETIME);

	static const GTypeValueTable timespan_value_table = {
		value_init: _value_init_int64,
		value_free: NULL,
		value_copy: _value_copy_int64,
		value_peek_pointer: NULL,
		collect_format: "q",
		collect_value: _value_collect_int64,
		lcopy_format: "p",
		lcopy_value: _value_lcopy_int64,
	};

	info.value_table = &timespan_value_table;
	GSDL_TYPE_TIMESPAN = g_type_fundamental_next();
	g_type_register_fundamental(GSDL_TYPE_TIMESPAN, g_intern_static_string("gsdltimespan"), &info, &finfo, 0);
	g_value_register_transform_func(GSDL_TYPE_TIMESPAN, G_TYPE_STRING, _value_transform_timespan_string);

	static const GTypeValueTable unichar_value_table = {
		value_init: _value_init_int64,
		value_free: NULL,
		value_copy: _value_copy_int64,
		value_peek_pointer: NULL,
		collect_format: "q",
		collect_value: _value_collect_int64,
		lcopy_format: "p",
		lcopy_value: _value_lcopy_int64,
	};

	info.value_table = &unichar_value_table;
	GSDL_TYPE_UNICHAR = g_type_fundamental_next();
	g_type_register_fundamental(GSDL_TYPE_UNICHAR, g_intern_static_string("gsdlunichar"), &info, &finfo, 0);
	g_value_register_transform_func(GSDL_TYPE_UNICHAR, G_TYPE_STRING, _value_transform_unichar_string);

	init_done = true;
}
