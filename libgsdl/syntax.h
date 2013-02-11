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

#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include <glib.h>

//> Error Definitions
/**
 * GSDL_SYNTAX_ERROR:
 *
 * Error domain for the SDL parser. Any errors from this domain will be in the %GSDLSyntaxError
 * enumeration.
 */
#define GSDL_SYNTAX_ERROR gsdl_syntax_error_quark()

/**
 * GSDLSyntaxError:
 * @GSDL_SYNTAX_ERROR_UNEXPECTED_CHAR: An unexpected character was found while reading the source file.
 * @GSDL_SYNTAX_ERROR_MISSING_DELIMITER: Did not find the end of a string or binary literal before
 *                                       the end of the file.
 * @GSDL_SYNTAX_ERROR_MALFORMED: Bad syntax; unexpected token in the input.
 * @GSDL_SYNTAX_ERROR_BAD_LITERAL: Bad formatting inside a literal, or out of range value.
 */
typedef enum {
	GSDL_SYNTAX_ERROR_UNEXPECTED_CHAR,
	GSDL_SYNTAX_ERROR_MISSING_DELIMITER,
	GSDL_SYNTAX_ERROR_MALFORMED,
	GSDL_SYNTAX_ERROR_BAD_LITERAL,
} GSDLSyntaxError;

extern GQuark gsdl_syntax_error_quark();

#endif
