#include "tokenizer.h"

#include <ctype.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tokenizer.h"

#define GROW_IF_NEEDED(str, i, alloc) if (i >= alloc) { alloc = alloc * 2 + 1; str = g_realloc(str, alloc); }

struct _GSDLTokenizer {
	GIOChannel *channel;
	gunichar *stringbuf;
	
	int line;
	int col;

	bool peek_avail;
	gunichar peeked;
};

GSDLTokenizer* gsdl_tokenizer_new(const char *filename, GError **err) {
	GSDLTokenizer* self = g_new(GSDLTokenizer, 1);
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
	self->stringbuf = g_utf8_to_ucs4(str, -1, NULL, NULL, err);

	if (!self->stringbuf) return NULL;

	self->channel = NULL;
	self->line = 1;
	self->col = 1;
	self->peek_avail = false;

	return self;
}

//> Utility Functions
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

	self->peek_avail = false;
}

static GSDLToken* _maketoken(GSDLTokenType type, int line, int col) {
	GSDLToken *result = g_slice_new0(GSDLToken);

	result->type = type;
	result->line = line;
	result->col = col;

	return result;
}

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

	char *alnum_part = output + i;

	while (_peek(self, &c, err) && c < 256 && (isalpha(c) || isdigit(c))) {
		GROW_IF_NEEDED(output = result->val, i + 1, length);

		_consume(self);
		output[i++] = (gunichar) c;
	}

	output[i] = '\0';

	if (strcasecmp("bd", alnum_part) == 0) {
		result->type = T_DECIMAL_END;
	} else if (strcasecmp("d", alnum_part) == 0) {
		result->type = T_D_NUMBER;
	} else if (strcasecmp("f", alnum_part) == 0) {
		result->type = T_FLOAT_END;
	} else if (strcasecmp("l", alnum_part) == 0) {
		result->type = T_LONGINTEGER;
	} else {
		// FIXME: Garbage
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

	return (err == NULL || *err == NULL);
}

static bool _tokenize_binary(GSDLTokenizer *self, GSDLToken *result, gunichar c, GError **err) {
	int length = 7;
	char *output = result->val = g_malloc(length);

	output[0] = c;
	int i = 1;

	while (_peek(self, &c, err) && c != ']' && c != EOF) {
		GROW_IF_NEEDED(output = result->val, i, length);

		_consume(self);
		output[i++] = (gunichar) c;
	}
	output[i] = '\0';

	return (err == NULL || *err == NULL);
}

static bool _tokenize_string(GSDLTokenizer *self, GSDLToken *result, gunichar c, GError **err) {
	int length = 7;
	char *output = result->val = g_malloc(length);

	output[0] = c;
	int i = 1;

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
			}
		} else {
			output[i++] = (gunichar) c;
		}
	}
	output[i] = '\0';

	return (err == NULL || *err == NULL);
}

static bool _tokenize_backquote_string(GSDLTokenizer *self, GSDLToken *result, gunichar c, GError **err) {
	int length = 7;
	char *output = result->val = g_malloc(length);

	output[0] = c;
	int i = 1;

	while (_peek(self, &c, err) && c != '`' && c != EOF) {
		GROW_IF_NEEDED(output = result->val, i, length);

		_consume(self);

		if (c == '\r') _read(self, &c, err);

		output[i++] = (gunichar) c;
	}
	output[i] = '\0';

	return (err == NULL || *err == NULL);
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
		return _tokenize_binary(self, *result, c, err);
	} else if (c == '"') {
		*result = _maketoken(T_STRING, line, col);
		if (!_tokenize_string(self, *result, c, err)) return false;

		return _read(self, &c, err);
	} else if (c == '`') {
		*result = _maketoken(T_STRING, line, col);
		if (!_tokenize_backquote_string(self, *result, c, err)) return false;

		return _read(self, &c, err);
	} else if (c == '\'') {
		*result = _maketoken(T_CHAR, line, col);
		(*result)->val = g_malloc0(1);

		if (c == '\\') {
			_read(self, &c, err);

			switch (c) {
				case 'n': (*result)->val[0] = '\n'; break;
				case 'r': (*result)->val[0] = '\r'; break;
				case 't': (*result)->val[0] = '\t'; break;
				case '"': (*result)->val[0] = '"'; break;
				case '\'': (*result)->val[0] = '\''; break;
				case '\\': (*result)->val[0] = '\\'; break;
			}
		} else {
			(*result)->val[0] = c;
		}

		return _read(self, &c, err);
	} else if (c == ' ' || c == '\t') {
		// Do nothing
		goto retry;
	} else {
		fprintf(stderr, "Invalid character '%s'(%d) at line %d, col %d", g_ucs4_to_utf8(&c, 1, NULL, NULL, NULL), c, line, col);
		return false;
	}
}
