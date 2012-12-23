#ifndef __TYPES_H__
#define __TYPES_H__

#include <glib-object.h>

//> Types
extern GType GSDL_TYPE_DECIMAL;
#define GSDL_GVALUE_HOLDS_DECIMAL(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DECIMAL))

extern void gsdl_gvalue_set_decimal(GValue *value, const gchar *contents);
extern const gchar* gsdl_gvalue_get_decimal(const GValue *value);

#endif
