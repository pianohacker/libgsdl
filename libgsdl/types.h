#ifndef __TYPES_H__
#define __TYPES_H__

#include <glib-object.h>

#define DEF_PROTO(suffix, upper_suffix, type)                  \
extern GType GSDL_TYPE_##upper_suffix;                         \
void gsdl_gvalue_set_##suffix(GValue *value, const type *src); \
const type* gsdl_gvalue_get_##suffix(const GValue *value);

DEF_PROTO(decimal, DECIMAL, gchar)
DEF_PROTO(date, DATE, GDate)
DEF_PROTO(datetime, DATETIME, GDateTime)

#define GSDL_GVALUE_HOLDS_DECIMAL(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DECIMAL))
#define GSDL_GVALUE_HOLDS_DATE(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DATE))
#define GSDL_GVALUE_HOLDS_DATETIME(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DATETIME))
#define GSDL_GVALUE_HOLDS_TIMESPAN(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_TIMESPAN))

#endif
