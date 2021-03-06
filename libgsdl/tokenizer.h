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

#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <glib.h>
#include <stdbool.h>
#include <stdio.h>

//> Types
/**
 * GSDLTokenType:
 * @T_EOF: A virtual token at the end of the input.*
 * @T_IDENTIFIER: An Unicode identifier, following the same rules as a Java identifier.
 * @T_NUMBER: A sequence of ASCII digits.
 * @T_LONGINTEGER: A number that was preceded by a . and followed by L or l.
 * @T_FLOAT_END: A number that was preceded by a . and followed by F or f.
 * @T_DOUBLE_END: A number that was preceded by a . and followed by D or d.
 * @T_DECIMAL_END: A number that was preceded by a . and followed by BD or bd.
 * @T_DATE_PART: A number that was followed by a /.
 * @T_TIME_PART: A number that was followed by a :.
 * @T_DAYS: A number that was followed by D: or d:.
 * @T_BOOLEAN: One of the keywords "true", "false", "on", or "off".
 * @T_NULL: The keyword "null".*
 * @T_STRING: A string enclosed by "" or ``.
 * @T_CHAR: A single UTF-8 character enclosed by ''.
 * @T_BINARY: A base64 binary literal enclosed by [], with all invalid characters removed.
 *
 * * These token types have no value, and their #GSDLToken.val field is undefined.
 */
typedef enum {
	T_EOF = EOF,
	T_IDENTIFIER = 256,
	T_NUMBER,
	T_LONGINTEGER,
	T_FLOAT_END,
	T_DOUBLE_END,
	T_DECIMAL_END,
	T_DATE_PART,
	T_TIME_PART,
	T_DAYS,
	T_BOOLEAN,
	T_NULL,
	T_STRING,
	T_CHAR,
	T_BINARY,
} GSDLTokenType;

/**
 * GSDLToken:
 * @type: The type of the token, either one of %GSDLTokenType or an ASCII character in the range
 *        0-255.
 * @line: The line where the token occurred.
 * @col: The column where the token occurred.
 * @val: Any string contents of the token, as a %NULL-terminated string. This is undefined for any
 *       single-character token, and %T_EOF and %T_NULL.
 */
typedef struct {
	GSDLTokenType type;

	guint line;
	guint col;

	char *val;
} GSDLToken;

typedef struct _GSDLTokenizer GSDLTokenizer;

//> Exported Functions
extern GSDLTokenizer* gsdl_tokenizer_new(const char *filename, GError **err);
extern GSDLTokenizer* gsdl_tokenizer_new_from_string(const char *str, GError **err);

extern bool gsdl_tokenizer_next(GSDLTokenizer *self, GSDLToken **token, GError **err);
extern char* gsdl_tokenizer_get_filename(GSDLTokenizer *self);

extern void gsdl_tokenizer_free(GSDLTokenizer *self);

extern char* gsdl_token_type_name(GSDLTokenType token_type);
extern void gsdl_token_free(GSDLToken *token);

#endif
