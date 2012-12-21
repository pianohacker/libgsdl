#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include <glib.h>

//> Error Definitions
#define GSDL_SYNTAX_ERROR gsdl_syntax_error_quark()

typedef enum {
	GSDL_SYNTAX_ERROR_UNEXPECTED_CHAR,
	GSDL_SYNTAX_ERROR_MISSING_DELIMITER,
	GSDL_SYNTAX_ERROR_MALFORMED,
	GSDL_SYNTAX_ERROR_BAD_LITERAL,
} GSDLSyntaxError;

extern GQuark gsdl_syntax_error_quark();

#endif
