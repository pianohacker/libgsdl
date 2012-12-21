#ifndef __SYNTAX_H__
#define __SYNTAX_H__

//> Error Definitions
#define GSDL_SYNTAX_ERROR gsdl_syntax_error_quark()

typedef enum {
	GSDL_SYNTAX_ERROR_UNEXPECTED_CHAR,
	GSDL_SYNTAX_ERROR_MISSING_DELIMITER,
	GSDL_SYNTAX_ERROR_MALFORMED,
} GSDLSyntaxError;

#endif
