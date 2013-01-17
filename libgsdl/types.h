#ifndef __TYPES_H__
#define __TYPES_H__

#include <glib-object.h>

#define DEF_PROTO(suffix, upper_suffix, type)                  \
extern GType GSDL_TYPE_##upper_suffix;                         \
void gsdl_gvalue_set_##suffix(GValue *value, const type *src); \
void gsdl_gvalue_take_##suffix(GValue *value, type *src); \
const type* gsdl_gvalue_get_##suffix(const GValue *value);

DEF_PROTO(binary, BINARY, GByteArray)
DEF_PROTO(decimal, DECIMAL, gchar)
DEF_PROTO(date, DATE, GDate)
DEF_PROTO(datetime, DATETIME, GDateTime)

extern GType GSDL_TYPE_TIMESPAN;
void gsdl_gvalue_set_timespan(GValue *value, const GTimeSpan src);
GTimeSpan gsdl_gvalue_get_timespan(const GValue *value);

extern GType GSDL_TYPE_UNICHAR;
void gsdl_gvalue_set_unichar(GValue *value, const gunichar src);
gunichar gsdl_gvalue_get_unichar(const GValue *value);

#define GSDL_GVALUE_HOLDS_BINARY(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_BINARY))
#define GSDL_GVALUE_HOLDS_DECIMAL(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DECIMAL))
#define GSDL_GVALUE_HOLDS_DATE(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DATE))
#define GSDL_GVALUE_HOLDS_DATETIME(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DATETIME))
#define GSDL_GVALUE_HOLDS_TIMESPAN(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_TIMESPAN))
#define GSDL_GVALUE_HOLDS_UNICHAR(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_UNICHAR))

#endif
