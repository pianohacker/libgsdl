#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <glib.h>
#include <stdbool.h>
#include <stdio.h>

//> Types
typedef enum {
	T_EOF = EOF,
	T_IDENTIFIER = 256,
	T_NUMBER,
	T_LONGINTEGER,
	T_FLOAT_END,
	T_DECIMAL_END,
	T_D_NUMBER,
	T_BOOLEAN,
	T_NULL,
	T_STRING,
	T_CHAR,
	T_BINARY,
} GSDLTokenType;

typedef struct {
	GSDLTokenType type;

	guint line;
	guint col;

	char *val;
} GSDLToken;

typedef struct _GSDLTokenizer GSDLTokenizer;

//> Error Definitions
#define GSDL_TOKENIZER_ERROR gsdl_tokenizer_error_quark()

typedef enum {
	GSDL_TOKENIZER_ERROR_UNEXPECTED_CHAR,
	GSDL_TOKENIZER_ERROR_MISSING_DELIMITER,
} GSDLTokenizerError;

extern GQuark gsdl_tokenizer_error_quark();

//> Exported Functions
extern GSDLTokenizer* gsdl_tokenizer_new(const char *filename, GError **err);
extern GSDLTokenizer* gsdl_tokenizer_new_from_string(const char *str, GError **err);

extern bool gsdl_tokenizer_next(GSDLTokenizer *self, GSDLToken **token, GError **err);

#endif
