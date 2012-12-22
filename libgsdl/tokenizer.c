#include <ctype.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "syntax.h"
#include "tokenizer.h"

//> Macros
#define FAIL_IF_ERR() if ((err != NULL) && (*err != NULL)) return false;
#define GROW_IF_NEEDED(str, i, alloc) if (i >= alloc) { alloc = alloc * 2 + 1; str = g_realloc(str, alloc); }
#define REQUIRE(expr) if (!expr) return false;

//> Internal Types
struct _GSDLTokenizer {
	char *filename;
	GIOChannel *channel;
	gunichar *stringbuf;
	
	int line;
	int col;

	bool peek_avail;
	gunichar peeked;
};

//> Static Data
static char *TOKEN_NAMES[12] = {
	"EOF",
	"identifier",
	"number",
	"long integer",
	"float suffix",
	"decimal suffix",
	"days/double suffix",
	"boolean",
	"null",
	"string",
	"character",
	"binary",
};

//> Public Functions
GSDLTokenizer* gsdl_tokenizer_new(const char *filename, GError **err) {
	GSDLTokenizer* self = g_new(GSDLTokenizer, 1);
	self->filename = g_strdup(filename);
	self->channel = g_io_channel_new_file(filename, "r", err);

	if (!self->channel) return NULL;

	self->stringbuf = NULL;
	self->line = 1;
	self->col = 1;
	self->peek_avail = false;

	return self;
}

GSDLTokenizer* gsdl_tokenizer_new_from_string(const char *str, GError **err) {
	GSDLTokenizer* self = g_new(GSDLTokenizer, 1);
	self->filename = "<string>";
	self->stringbuf = g_utf8_to_ucs4(str, -1, NULL, NULL, err);

	if (!self->stringbuf) return NULL;

	self->channel = NULL;
	self->line = 1;
	self->col = 1;
	self->peek_avail = false;

	return self;
}

char* gsdl_tokenizer_get_filename(GSDLTokenizer *self) {
	return self->filename;
}

void gsdl_tokenizer_free(GSDLTokenizer *self) {
	g_free(self->filename);

	if (self->channel) {
		g_io_channel_shutdown(self->channel, FALSE, NULL);
		g_io_channel_unref(self->channel);
	}

	if (self->stringbuf) g_free(self->stringbuf);

	g_free(self);
}

extern char* gsdl_token_type_name(GSDLTokenType token_type) {
	static char buffer[4] = "' '";

	if (0 <= token_type && token_type < 256) {
		buffer[1] = token_type;
		return buffer;
	} else {
		return TOKEN_NAMES[token_type == EOF ? 0 : (token_type - 255)];
	}
}

extern void gsdl_token_free(GSDLToken *token) {
	if (token->val) g_free(token->val);

	g_slice_free(GSDLToken, token);
}

//> Internal Functions
static bool _read(GSDLTokenizer *self, gunichar *result, GError **err) {
	if (self->peek_avail) {
		*result = self->peeked;
		self->peek_avail = false;
	} else if (self->stringbuf) {
		if (!*self->stringbuf) {
			self->stringbuf = NULL;
			*result = EOF;
		} else {
			*result = *(self->stringbuf++);
		}

		return true;
	} else {
		if (G_UNLIKELY(!self->channel)) return false;

		switch (g_io_channel_read_unichar(self->channel, result, err)) {
			case G_IO_STATUS_ERROR:
				self->channel = NULL;
				self->peek_avail = false;
				return false;
			case G_IO_STATUS_EOF:
				self->peek_avail = false;
				*result = EOF;
				g_io_channel_shutdown(self->channel, FALSE, NULL);
				g_io_channel_unref(self->channel);
				self->channel = NULL;
				return true;
			case G_IO_STATUS_AGAIN:
			case G_IO_STATUS_NORMAL:
				break;
		}
	}

	if (*result == '\n') {
		self->line++;
		self->col = 1;
	} else {
		self->col++;
	}

	return true;
}

static bool _peek(GSDLTokenizer *self, gunichar *result, GError **err) {
	if (!self->peek_avail) {
		if (self->stringbuf) {
			if (*self->stringbuf) {
				self->peeked = *(self->stringbuf++);
			} else {
				self->stringbuf = NULL;
				self->peeked = EOF;
			}
			self->peek_avail = true;
		} else {
			if (self->channel == NULL) return false;

			switch (g_io_channel_read_unichar(self->channel, &(self->peeked), err)) {
				case G_IO_STATUS_ERROR:
					self->channel = NULL;
					self->peek_avail = false;
					return false;
				case G_IO_STATUS_EOF:
					self->peeked = EOF;
					g_io_channel_shutdown(self->channel, FALSE, NULL);
					g_io_channel_unref(self->channel);
					self->channel = NULL;
				case G_IO_STATUS_AGAIN:
				case G_IO_STATUS_NORMAL:
				default:
					self->peek_avail = true;
			}
		}
	}

	*result = self->peeked;
	return true;
}

static void _consume(GSDLTokenizer *self) {
	g_assert(self->peek_avail);

	gunichar result;
	_read(self, &result, NULL);
}

static GSDLToken* _maketoken(GSDLTokenType type, int line, int col) {
	GSDLToken *result = g_slice_new0(GSDLToken);

	result->type = type;
	result->line = line;
	result->col = col;

	return result;
}

static void _set_error(GError **err, GSDLTokenizer *self, GSDLSyntaxError err_type, char *msg) {
	g_set_error(err,
		GSDL_SYNTAX_ERROR,
		err_type,
		"%s in %s, line %d, column %d",
		msg,
		self->filename,
		self->line,
		self->col
	);
}

//> Sub-tokenizers
static bool _tokenize_number(GSDLTokenizer *self, GSDLToken *result, gunichar c, GError **err) {
	int length = 7;
	char *output = result->val = g_malloc(length);

	output[0] = c;
	int i = 1;

	while (_peek(self, &c, err) && c < 256 && isdigit(c)) {
		GROW_IF_NEEDED(output = result->val, i + 1, length);

		_consume(self);
		output[i++] = (gunichar) c;
	}

	FAIL_IF_ERR();

	char *alnum_part = output + i;

	while (_peek(self, &c, err) && c < 256 && (isalpha(c) || isdigit(c))) {
		GROW_IF_NEEDED(output = result->val, i + 1, length);

		_consume(self);
		output[i++] = (gunichar) c;
	}

	FAIL_IF_ERR();

	output[i] = '\0';

	if (*alnum_part == '\0') {
		// Just a T_NUMBER
	} else if (strcasecmp("bd", alnum_part) == 0) {
		result->type = T_DECIMAL_END;
	} else if (strcasecmp("d", alnum_part) == 0) {
		result->type = T_D_NUMBER;
	} else if (strcasecmp("f", alnum_part) == 0) {
		result->type = T_FLOAT_END;
	} else if (strcasecmp("l", alnum_part) == 0) {
		result->type = T_LONGINTEGER;
	} else {
		_set_error(err, self, GSDL_SYNTAX_ERROR_UNEXPECTED_CHAR, g_strdup_printf("Unexpected number suffix: \"%s\"", alnum_part));
		return false;
	}

	*alnum_part = '\0';

	return true;
}

static bool _tokenize_identifier(GSDLTokenizer *self, GSDLToken *result, gunichar c, GError **err) {
	int length = 7;
	char *output = result->val = g_malloc(length);
	GUnicodeType type;

	int i = g_unichar_to_utf8(c, output);

	while (_peek(self, &c, err) && (g_unichar_isalpha(c) || g_unichar_isdigit(c) || (type = g_unichar_type(c)) == G_UNICODE_CURRENCY_SYMBOL || type == G_UNICODE_CONNECT_PUNCTUATION || type == G_UNICODE_LETTER_NUMBER || type == G_UNICODE_SPACING_MARK || type == G_UNICODE_NON_SPACING_MARK)) {
		GROW_IF_NEEDED(output = result->val, i + 5, length);

		_consume(self);
		i += g_unichar_to_utf8(c, output + i);
	}

	FAIL_IF_ERR();
	output[i] = '\0';

	if (
			strcmp(output, "true") == 0 ||
			strcmp(output, "on") == 0 ||
			strcmp(output, "false") == 0 ||
			strcmp(output, "off") == 0) {
		result->type = T_BOOLEAN;
	} else if (strcmp(output, "null") == 0) {
		result->type = T_NULL;
	}

	return true;
}

static bool _tokenize_binary(GSDLTokenizer *self, GSDLToken *result, GError **err) {
	int length = 7;
	gunichar c;
	char *output = result->val = g_malloc(length);
	int i = 0;

	while (_peek(self, &c, err) && c != ']' && c != EOF) {
		_consume(self);

		if (c < 256 && (isalpha((char) c) || isdigit((char) c) || strchr("+/=", (char) c))) {
			GROW_IF_NEEDED(output = result->val, i, length);
			output[i++] = (gunichar) c;
		}
	}

	FAIL_IF_ERR();
	output[i] = '\0';

	return true;
}

static bool _tokenize_string(GSDLTokenizer *self, GSDLToken *result, GError **err) {
	int length = 7;
	gunichar c;
	char *output = result->val = g_malloc(length);
	int i = 0;

	while (_peek(self, &c, err) && c != '"' && c != EOF) {
		GROW_IF_NEEDED(output = result->val, i, length);

		_consume(self);

		if (c == '\\') {
			_read(self, &c, err);

			switch (c) {
				case 'n': output[i++] = '\n'; break;
				case 'r': output[i++] = '\r'; break;
				case 't': output[i++] = '\t'; break;
				case '"': output[i++] = '"'; break;
				case '\'': output[i++] = '\"'; break;
				case '\\': output[i++] = '\\'; break;
				case '\r':
					_read(self, &c, err);
				case '\n':
					output[i++] = '\n';
					while (_peek(self, &c, err) && (c == ' ' || c == '\t')) _consume(self);
					break;
				default:
					i += g_unichar_to_utf8(c, output + i);
			}
		} else {
			i += g_unichar_to_utf8(c, output + i);
		}
	}

	FAIL_IF_ERR();
	output[i] = '\0';

	return true;
}

static bool _tokenize_backquote_string(GSDLTokenizer *self, GSDLToken *result, GError **err) {
	int length = 7;
	gunichar c;
	char *output = result->val = g_malloc(length);
	int i = 0;

	while (_peek(self, &c, err) && c != '`' && c != EOF) {
		GROW_IF_NEEDED(output = result->val, i, length);

		_consume(self);

		if (c == '\r') _read(self, &c, err);

		i += g_unichar_to_utf8(c, output + i);
	}

	FAIL_IF_ERR();
	output[i] = '\0';

	return true;
}

bool gsdl_tokenizer_next(GSDLTokenizer *self, GSDLToken **result, GError **err) {
	gunichar c, nc;
	int line;
	int col;

	retry:
	line = self->line;
	col = self->col;
	if (!_read(self, &c, err)) return false;

	if (G_UNLIKELY(c == EOF)) {
		*result = _maketoken(T_EOF, line, col);
		return true;
	} else if (c == '\r') {
		if (_peek(self, &c, err) && c == '\n') _consume(self);

		*result = _maketoken('\n', line, col);
		FAIL_IF_ERR();

		return true;
	} else if ((c == '/' && _peek(self, &nc, err) && nc == '/') || (c == '-' && _peek(self, &nc, err) && nc == '-') || c == '#') {
		if (c != '#') _consume(self);
		while (_peek(self, &c, err) && !(c == '\n' || c == EOF)) _consume(self);

		goto retry;
	} else if (c < 256 && strchr("-:./{}=\n", (char) c)) {
		*result = _maketoken(c, line, col);
		return true;
	} else if (c < 256 && isdigit((char) c)) {
		*result = _maketoken(T_NUMBER, line, col);
		return _tokenize_number(self, *result, c, err);
	} else if (g_unichar_isalpha(c) || g_unichar_type(c) == G_UNICODE_CONNECT_PUNCTUATION || g_unichar_type(c) == G_UNICODE_CURRENCY_SYMBOL) {
		*result = _maketoken(T_IDENTIFIER, line, col);
		return _tokenize_identifier(self, *result, c, err);
	} else if (c == '[') {
		*result = _maketoken(T_BINARY, line, col);
		if (!_tokenize_binary(self, *result, err)) return false;

		REQUIRE(_read(self, &c, err));
		if (c == ']') {
			return true;
		} else {
			_set_error(err,
				self,
				GSDL_SYNTAX_ERROR_MISSING_DELIMITER,
				"Missing ']'"
			);
			return false;
		}
	} else if (c == '"') {
		*result = _maketoken(T_STRING, line, col);
		if (!_tokenize_string(self, *result, err)) return false;

		REQUIRE(_read(self, &c, err));
		if (c == '"') {
			return true;
		} else {
			_set_error(err,
				self,
				GSDL_SYNTAX_ERROR_MISSING_DELIMITER,
				"Missing '\"'"
			);
			return false;
		}
	} else if (c == '`') {
		*result = _maketoken(T_STRING, line, col);
		if (!_tokenize_backquote_string(self, *result, err)) return false;

		REQUIRE(_read(self, &c, err));
		if (c == '`') {
			return true;
		} else {
			_set_error(err,
				self,
				GSDL_SYNTAX_ERROR_MISSING_DELIMITER,
				"Missing '`'"
			);
			return false;
		}
	} else if (c == '\'') {
		*result = _maketoken(T_CHAR, line, col);
		(*result)->val = g_malloc0(4);

		_read(self, &c, err);

		if (c == '\\') {
			_read(self, &c, err);

			switch (c) {
				case 'n': c = '\n'; break;
				case 'r': c = '\r'; break;
				case 't': c = '\t'; break;
				case '"': c = '"'; break;
				case '\'': c = '\''; break;
				case '\\': c = '\\'; break;
			}
		}

		g_unichar_to_utf8(c, (*result)->val); 

		REQUIRE(_read(self, &c, err));
		if (c == '\'') {
			return true;
		} else {
			_set_error(err,
				self,
				GSDL_SYNTAX_ERROR_MISSING_DELIMITER,
				"Missing \"'\""
			);
			return false;
		}
	} else if (c == '\\' && _peek(self, &nc, err) && (nc == '\r' || nc == '\n')) {
		_consume(self);

		if (c == '\r') _read(self, &c, err);

		goto retry;
	} else if (c == ' ' || c == '\t') {
		// Do nothing
		goto retry;
	} else {
		_set_error(err,
			self,
			GSDL_SYNTAX_ERROR_UNEXPECTED_CHAR,
		   	g_strdup_printf("Invalid character '%s'(%d)", g_ucs4_to_utf8(&c, 1, NULL, NULL, NULL), c)
		);
		return false;
	}
}
