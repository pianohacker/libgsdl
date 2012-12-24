#include <glib.h>
#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include <stdbool.h>

#include "types.h"

// Large files of this are liberally copied from glib's gvaluetypes.c.
// Many thanks to their original authors.

#define DEF_GET_SET(suffix, upper_suffix, dup_function, type)            \
                                                                         \
void gsdl_gvalue_set_##suffix(GValue *value, const type *src) {          \
	gpointer new_val;                                                    \
                                                                         \
	g_return_if_fail(GSDL_GVALUE_HOLDS_##upper_suffix(value));           \
                                                                         \
	new_val = dup_function(src);                                         \
                                                                         \
	if (value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS) {               \
		value->data[1].v_uint = 0;                                       \
	} else {                                                             \
		g_free(value->data[0].v_pointer);                                \
	}                                                                    \
                                                                         \
	value->data[0].v_pointer = new_val;                                  \
}                                                                        \
                                                                         \
const type* gsdl_gvalue_get_##suffix(const GValue *value) {              \
	g_return_val_if_fail(GSDL_GVALUE_HOLDS_##upper_suffix(value), NULL); \
                                                                         \
	return (type*) value->data[0].v_pointer;                             \
}

#define DEF_POINTER_VALUE(suffix, upper_suffix, dup_function)                                       \
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
		value_free: _value_free_pointer,                                                                            \
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

static inline gpointer _g_date_dup(gconstpointer src) {
	return g_memdup(src, sizeof(GDate));
}

static inline gpointer _g_datetime_dup(gconstpointer src) {
	return g_date_time_add((GDateTime*) src, 0);
}

DEF_GET_SET(decimal, DECIMAL, g_strdup, gchar)
DEF_GET_SET(date, DATE, _g_date_dup, GDate)
DEF_GET_SET(datetime, DATETIME, _g_datetime_dup, GDateTime)

GType GSDL_TYPE_TIMESPAN;
DEF_POINTER_VALUE(decimal, DECIMAL, g_strdup);
DEF_POINTER_VALUE(date, DATE, _g_date_dup);
DEF_POINTER_VALUE(datetime, DATETIME, _g_datetime_dup);

static void _value_init_pointer(GValue *value) {
	value->data[0].v_pointer = NULL;
}

static void _value_free_pointer(GValue *value) {
	if (!(value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS)) {
		g_free(value->data[0].v_pointer);
	}
}

static gpointer _value_peek_pointer(const GValue *value) {
	return value->data[0].v_pointer;
}

static void _value_transform_decimal_string(const GValue *src_value, GValue *dest_value) {
	dest_value->data[0].v_pointer = g_strdup(src_value->data[0].v_pointer);
}

static void _value_transform_date_string(const GValue *src_value, GValue *dest_value) {
	GDate *src = (GDate*) src_value->data[0].v_pointer;
	dest_value->data[0].v_pointer = g_strdup_printf("%04d-%02d-%02d", src->year, src->month, src->day);
}

static void _value_transform_datetime_string(const GValue *src_value, GValue *dest_value) {
	dest_value->data[0].v_pointer = g_date_time_format((GDateTime*) src_value->data[0].v_pointer, "%FT%R:%s+%z");
}

static void _value_init_timespan(GValue *value) {
}

static void _value_copy_timespan(const GValue *src_value, GValue *dest_value) {
	dest_value->data[0].v_int64 = src_value->data[0].v_int64;
}

static gchar* _value_collect_timespan(
		GValue *value,
		guint n_collect_values,
		GTypeCValue *collect_values,
		guint collect_flags
	) {

	value->data[0].v_int64 = collect_values[0].v_int64;

	return NULL;
}

static gchar* _value_lcopy_timespan(
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
	const GTypeFundamentalInfo finfo = { G_TYPE_FLAG_DERIVABLE, };                                                  \

	REGISTER_POINTER_VALUE(decimal, DECIMAL);
	REGISTER_POINTER_VALUE(date, DATE);
	REGISTER_POINTER_VALUE(datetime, DATETIME);

	static const GTypeValueTable timespan_value_table = {
		value_init: _value_init_timespan,
		value_free: NULL,
		value_copy: _value_copy_timespan,
		value_peek_pointer: NULL,
		collect_format: "q",
		collect_value: _value_collect_timespan,
		lcopy_format: "p",
		lcopy_value: _value_lcopy_timespan,
	};

	info.value_table = &timespan_value_table;
	GSDL_TYPE_TIMESPAN = g_type_fundamental_next();
	g_type_register_fundamental(GSDL_TYPE_TIMESPAN, g_intern_static_string("gsdltimespan"), &info, &finfo, 0);
	g_value_register_transform_func(GSDL_TYPE_TIMESPAN, G_TYPE_STRING, _value_transform_timespan_string);

	init_done = true;
}
