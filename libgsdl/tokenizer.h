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

//> Exported Functions
extern GSDLTokenizer* gsdl_tokenizer_new(const char *filename, GError **err);
extern GSDLTokenizer* gsdl_tokenizer_new_from_string(const char *str, GError **err);

extern bool gsdl_tokenizer_next(GSDLTokenizer *self, GSDLToken **token, GError **err);
extern char* gsdl_tokenizer_get_filename(GSDLTokenizer *self);

extern void gsdl_tokenizer_free(GSDLTokenizer *self);

extern char* gsdl_token_type_name(GSDLTokenType token_type);
extern void gsdl_token_free(GSDLToken *token);

#endif
