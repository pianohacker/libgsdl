#include <glib.h>
#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include <stdbool.h>

#include "types.h"

// Large files of this are liberally copied from glib's gvaluetypes.c.
// Many thanks to their original authors.

GType GSDL_TYPE_DECIMAL;

void gsdl_gvalue_set_decimal(GValue *value, const gchar *v_string) {
	gchar *new_val;

	g_return_if_fail(GSDL_GVALUE_HOLDS_DECIMAL(value));

	new_val = g_strdup (v_string);

	if (value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS) {
		value->data[1].v_uint = 0;
	} else {
		g_free (value->data[0].v_pointer);
	}

	value->data[0].v_pointer = new_val;
}

const gchar* gsdl_gvalue_get_decimal(const GValue *value) {
	g_return_val_if_fail(GSDL_GVALUE_HOLDS_DECIMAL(value), NULL);

	return value->data[0].v_pointer;
}

static void _value_init_decimal(GValue *value) {
	value->data[0].v_pointer = NULL;
}

static void _value_free_decimal(GValue *value) {
	if (!(value->data[1].v_uint & G_VALUE_NOCOPY_CONTENTS)) {
		g_free(value->data[0].v_pointer);
	}
}

static void _value_copy_decimal(const GValue *src_value, GValue *dest_value) {
	dest_value->data[0].v_pointer = g_strdup(src_value->data[0].v_pointer);
}

static gchar* _value_collect_decimal(
		GValue *value,
		guint n_collect_values,
		GTypeCValue *collect_values,
		guint collect_flags
	) {

	if (!collect_values[0].v_pointer) {
		value->data[0].v_pointer = NULL;
	} else if (collect_flags & G_VALUE_NOCOPY_CONTENTS) {
		value->data[0].v_pointer = collect_values[0].v_pointer;
		value->data[1].v_uint = G_VALUE_NOCOPY_CONTENTS;
	} else {
		value->data[0].v_pointer = g_strdup(collect_values[0].v_pointer);
	}

	return NULL;
}

static gchar* _value_lcopy_decimal(
		const GValue *value,
		guint n_collect_values,
		GTypeCValue *collect_values,
		guint collect_flags
	) {

	gchar **string_p = collect_values[0].v_pointer;

	if (!string_p) {
		return g_strdup_printf("value location for `%s' passed as NULL", G_VALUE_TYPE_NAME(value));
	}

	if (!value->data[0].v_pointer) {
		*string_p = NULL;
	} else if (collect_flags & G_VALUE_NOCOPY_CONTENTS) {
		*string_p = value->data[0].v_pointer;
	} else {
		*string_p = g_strdup (value->data[0].v_pointer);
	}

	return NULL;
}

static gpointer _value_peek_decimal(const GValue *value) {
	return value->data[0].v_pointer;
}

static void _value_transform_decimal_string(const GValue *src_value, GValue *dest_value) {
	dest_value->data[0].v_pointer = g_strdup(src_value->data[0].v_pointer);
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
	
	static const GTypeValueTable decimal_value_table = {
		value_init: _value_init_decimal,
		value_free: _value_free_decimal,
		value_copy: _value_copy_decimal,
		value_peek_pointer: _value_peek_decimal,
		collect_format: "p",
		collect_value: _value_collect_decimal,
		lcopy_format: "p",
		lcopy_value: _value_lcopy_decimal,
	};
	const GTypeFundamentalInfo finfo = { G_TYPE_FLAG_DERIVABLE, };

	info.value_table = &decimal_value_table;
	GSDL_TYPE_DECIMAL = g_type_fundamental_next();
	g_type_register_fundamental(GSDL_TYPE_DECIMAL, g_intern_static_string("gsdldecimal"), &info, &finfo, 0);
	g_value_register_transform_func(GSDL_TYPE_DECIMAL, G_TYPE_STRING, _value_transform_decimal_string);

	init_done = true;
}
