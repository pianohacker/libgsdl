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

/**
 * SECTION:gsdl-types
 * @short_description: Additional primitive types, to give full coverage of SDL's.
 *
 * This module adds binary, decimal, date/time, timespan and Unicode character values as primitive
 * %GValue types, so the parser can output simply %GValues.
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#include <glib-object.h>

extern GType GSDL_TYPE_BINARY;                         
/**
 * gsdl_gvalue_set_binary:
 * @value: The value to update.
 * @src: The %GByteArray to copy in.
 *
 * Sets a GValue to contain the binary data in @src.
 */
void gsdl_gvalue_set_binary(GValue *value, const GByteArray *src); 
void gsdl_gvalue_take_binary(GValue *value, GByteArray *src); 
const GByteArray* gsdl_gvalue_get_binary(const GValue *value);

extern GType GSDL_TYPE_DECIMAL;                         
/**
 * gsdl_gvalue_set_decimal:
 * @value: The value to update.
 * @src: The string to copy in.
 *
 * Sets a GValue to contain the decimal in @src, represented as a simple string.
 */
void gsdl_gvalue_set_decimal(GValue *value, const gchar *src); 
void gsdl_gvalue_take_decimal(GValue *value, gchar *src); 
const gchar* gsdl_gvalue_get_decimal(const GValue *value);

extern GType GSDL_TYPE_DATE;                         
/**
 * gsdl_gvalue_set_date:
 * @value: The value to update.
 * @src: The %GDate to copy in.
 *
 * Sets a GValue to contain the date in @src.
 */
void gsdl_gvalue_set_date(GValue *value, const GDate *src); 
void gsdl_gvalue_take_date(GValue *value, GDate *src); 
const GDate* gsdl_gvalue_get_date(const GValue *value);

extern GType GSDL_TYPE_DATETIME;
/**
 * gsdl_gvalue_set_datetime:
 * @value: The value to update.
 * @src: The %GDateTime to copy in.
 *
 * Sets a GValue to contain the date/time in @src.
 */
void gsdl_gvalue_set_datetime(GValue *value, const GDateTime *src); 
void gsdl_gvalue_take_datetime(GValue *value, GDateTime *src); 
const GDateTime* gsdl_gvalue_get_datetime(const GValue *value);

extern GType GSDL_TYPE_TIMESPAN;
/**
 * gsdl_gvalue_set_timespan:
 * @value: The value to update.
 * @src: The %GTimeSpan to copy in.
 *
 * Sets a GValue to contain the timespan in @src.
 */
void gsdl_gvalue_set_timespan(GValue *value, const GTimeSpan src);
GTimeSpan gsdl_gvalue_get_timespan(const GValue *value);

extern GType GSDL_TYPE_UNICHAR;
/**
 * gsdl_gvalue_set_unichar:
 * @value: The value to update.
 * @src: The %gunichar to copy in.
 *
 * Sets a GValue to contain the Unicode character in @src.
 */
void gsdl_gvalue_set_unichar(GValue *value, const gunichar src);
gunichar gsdl_gvalue_get_unichar(const GValue *value);

#define GSDL_GVALUE_HOLDS_BINARY(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_BINARY))
#define GSDL_GVALUE_HOLDS_DECIMAL(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DECIMAL))
#define GSDL_GVALUE_HOLDS_DATE(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DATE))
#define GSDL_GVALUE_HOLDS_DATETIME(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_DATETIME))
#define GSDL_GVALUE_HOLDS_TIMESPAN(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_TIMESPAN))
#define GSDL_GVALUE_HOLDS_UNICHAR(value) (G_TYPE_CHECK_VALUE_TYPE((value), GSDL_TYPE_UNICHAR))

#endif
