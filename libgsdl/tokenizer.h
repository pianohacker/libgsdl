#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <glib.h>
#include <stdbool.h>
#include <stdio.h>

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

	size_t len;
	char *val;
} GSDLToken;

typedef struct _GSDLTokenizer GSDLTokenizer;

extern GSDLTokenizer* gsdl_tokenizer_new(const char *filename, GError **err);
extern GSDLTokenizer* gsdl_tokenizer_new_from_string(const char *str, GError **err);
extern bool gsdl_tokenizer_next(GSDLTokenizer *self, GSDLToken **token, GError **err);

#endif
