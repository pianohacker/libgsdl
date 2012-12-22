#include <glib.h>
#include <glib-object.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "syntax.h"
#include "tokenizer.h"

struct _GSDLParserContext {
	GSDLTokenizer *tokenizer;
	GSDLToken *peek_token;

	GSDLParser *parser;
	gpointer user_data;

	GSList *parser_stack;
	GSList *data_stack;
};

#define EXPECT(...) if (!_expect(self, token, __VA_ARGS__, 0)) return false;
#define MAYBE_CALLBACK(callback, ...) if (callback) callback(__VA_ARGS__)
#define REQUIRE(expr) if (!expr) return false;

GSDLParserContext* gsdl_parser_context_new(GSDLParser *parser, gpointer user_data) {
	GSDLParserContext *self = g_new0(GSDLParserContext, 1);

	self->parser = parser;
	self->user_data = user_data;

	return self;
}

void gsdl_parser_context_push(GSDLParserContext *self, GSDLParser *parser, gpointer user_data) {
	self->parser_stack = g_slist_prepend(self->parser_stack, self->parser);
	self->data_stack = g_slist_prepend(self->data_stack, self->user_data);

	self->parser = parser;
	self->user_data = user_data;
}

gpointer gsdl_parser_context_pop(GSDLParserContext *self) {
	g_assert(self->parser_stack != NULL && self->data_stack != NULL);

	gpointer prev_data = self->user_data;

	self->parser = self->parser_stack->data;
	self->user_data = self->data_stack->data;

	self->parser_stack = self->parser_stack->next;
	self->data_stack = self->data_stack->next;

	return prev_data;
}

static bool _read(GSDLParserContext *self, GSDLToken **token) {
	if (self->peek_token) {
		GSDLToken *result = self->peek_token;
		self->peek_token = NULL;
		*token = result;

		return true;
	} else {
		GError *error = NULL;

		if (!gsdl_tokenizer_next(self->tokenizer, token, &error)) {
			MAYBE_CALLBACK(self->parser->error, self, error, self->user_data);
			return false;
		} else {
			return true;
		}
	}
}

static bool _peek(GSDLParserContext *self, GSDLToken **token) {
	if (self->peek_token) {
		*token = self->peek_token;

		return true;
	} else {
		GError *error = NULL;

		if (!gsdl_tokenizer_next(self->tokenizer, &(self->peek_token), &error)) {
			MAYBE_CALLBACK(self->parser->error, self, error, self->user_data);
			return false;
		} else {
			*token = self->peek_token;
			return true;
		}
	}
}

static void _consume(GSDLParserContext *self) {
	g_assert(self->peek_token != NULL);

	self->peek_token = NULL;
}

static void _error(GSDLParserContext *self, GSDLToken *token, GSDLSyntaxError err_type, char *msg) {
	GError *err = NULL;
	g_set_error(&err,
		GSDL_SYNTAX_ERROR,
		err_type,
		"%s in %s, line %d, column %d",
		msg,
		gsdl_tokenizer_get_filename(self->tokenizer),
		token->line,
		token->col
	);
	MAYBE_CALLBACK(self->parser->error, self, err, self->user_data);
}

static bool _expect(GSDLParserContext *self, GSDLToken *token, ...) {
	va_list args;
	GSDLTokenType type;

	va_start(args, token);

	while (type = va_arg(args, GSDLTokenType), type != 0 && token->type != type);

	va_end(args);

	if (type == 0) {
		GString *err_string = g_string_new("");
		g_string_sprintf(err_string, "Unexpected %s, expected one of: ", gsdl_token_type_name(token->type));

		va_start(args, token);
		type = va_arg(args, GSDLTokenType);

		g_string_append(err_string, gsdl_token_type_name(type));
		while (type = va_arg(args, GSDLTokenType), type != 0) {
			g_string_append(err_string, ", ");
			g_string_append(err_string, gsdl_token_type_name(type));
		}

		va_end(args);

		_error(
			self,
			token, 
			GSDL_SYNTAX_ERROR_MALFORMED,
			err_string->str
		);

		g_string_free(err_string, TRUE);

		return false;
	} else {
		return true;
	}
}

//> Parser Functions
static bool _token_is_value(GSDLToken *token) {
	switch (token->type) {
		case T_NUMBER:
		case T_LONGINTEGER:
		case T_D_NUMBER:
		case T_BOOLEAN:
		case T_NULL:
		case T_STRING:
		case T_CHAR:
		case T_BINARY:
			return true;
		default:
			return false;
	}
}

static bool _parse_value(GSDLParserContext *self, GValue *value) {
	char *end;
	GSDLToken *token;
	REQUIRE(_read(self, &token));

	switch (token->type) {
		case T_LONGINTEGER:
			g_value_init(value, G_TYPE_LONG);
			g_value_set_long(value, strtol(token->val, &end, 10));

			if (*end) {
				_error(self, token, GSDL_SYNTAX_ERROR_BAD_LITERAL, "Long integer out of range");

				return false;
			}

			g_free(token);
			return true;

		case T_BOOLEAN:
			g_value_init(value, G_TYPE_BOOLEAN);

			if (strcmp(token->val, "true") == 0 || strcmp(token->val, "on") == 0) {
				g_value_set_boolean(value, TRUE);
			} else if (strcmp(token->val, "false") == 0 || strcmp(token->val, "off") == 0) {
				g_value_set_boolean(value, FALSE);
			}

			g_free(token);
			return true;

		case T_NULL:
			return true;

		case T_STRING:
			g_value_init(value, G_TYPE_STRING);
			g_value_set_string(value, token->val);

			g_free(token);
			return true;

		default:
			g_return_val_if_reached(false);
	}
}

static bool _parse_tag(GSDLParserContext *self) {
	GSDLToken *first, *token;
	char *name = g_strdup("contents");

	GArray *values = g_array_new(TRUE, FALSE, sizeof(GValue));

	GArray *attr_names = g_array_new(TRUE, FALSE, sizeof(gchar*));
	GArray *attr_values = g_array_new(TRUE, FALSE, sizeof(GValue));

	REQUIRE(_peek(self, &first));

	if (first->type == T_IDENTIFIER) {
		_consume(self);

		REQUIRE(_peek(self, &token));

		if (token->type == '=') {
			_error(
				self,
				first,
				GSDL_SYNTAX_ERROR_MALFORMED,
				"At least one value required for an anonymous tag"
			);

			return false;
		}

		g_free(name);
		name = g_strdup(first->val);
		gsdl_token_free(first);
	} else {
		token = first;

		EXPECT(T_IDENTIFIER, T_NUMBER, T_LONGINTEGER, T_D_NUMBER, T_BOOLEAN, T_NULL, T_STRING, T_CHAR, T_BINARY);
	}

	bool peek_success = true;

	while ((_peek(self, &token) || (peek_success = false)) && _token_is_value(token)) {
		GValue value = G_VALUE_INIT;
		REQUIRE(_parse_value(self, &value));
		g_array_append_val(values, value);
	}
	REQUIRE(peek_success);

	while ((_peek(self, &token) || (peek_success = false)) && token->type == T_IDENTIFIER) {
		_consume(self);
		char *contents = g_strdup(token->val);
		g_array_append_val(attr_names, contents);
		gsdl_token_free(token);

		GValue value = G_VALUE_INIT;
		REQUIRE(_parse_value(self, &value));
		g_array_append_val(attr_values, value);
	}
	REQUIRE(peek_success);

	GError *err = NULL;
	MAYBE_CALLBACK(self->parser->start_tag,
		self,
		name,
		(GValue**) &values->data,
		(gchar**) attr_names->data,
		(GValue**) &attr_values->data,
		self->user_data,
		&err
	);
	if (err) {
		MAYBE_CALLBACK(self->parser->error, self, err, self->user_data);
		return false;
	}

	REQUIRE(_peek(self, &token));

	if (token->type == '{') {
		_consume(self);
		gsdl_token_free(token);

		while ((_peek(self, &token) || (peek_success = false)) && token->type != '}') {
			REQUIRE(_parse_tag(self));
			REQUIRE(_peek(self, &token));
			EXPECT('\n', '}');

			if (token->type == '\n') {
				_consume(self);
				gsdl_token_free(token);
			}
		}

		EXPECT('}');
		_consume(self);
		gsdl_token_free(token);
	}

	err = NULL;
	MAYBE_CALLBACK(self->parser->end_tag,
		self,
		name,
		self->user_data,
		&err
	);
	if (err) {
		MAYBE_CALLBACK(self->parser->error, self, err, self->user_data);
		return false;
	}

	return true;
}

static bool _parse(GSDLParserContext *self) {
	/*_types_init();*/

	GSDLToken *token;
	for (;;) {
		REQUIRE(_peek(self, &token));

		if (token->type == T_EOF) {
			break;
		} else if (token->type == '\n') {
			_consume(self);
			continue;
		} else {
			REQUIRE(_parse_tag(self));
			REQUIRE(_read(self, &token));
			EXPECT('\n', T_EOF);

			if (token->type == T_EOF) break;

			gsdl_token_free(token);
		}
	}

	return true;
}

bool gsdl_parser_context_parse_file(GSDLParserContext *self, const char *filename) {
	GError *err = NULL;
	self->tokenizer = gsdl_tokenizer_new(filename, &err);

	if (!self->tokenizer) {
		MAYBE_CALLBACK(self->parser->error, self, err, self->user_data);
		return false;
	}

	return _parse_tag(self);
}

bool gsdl_parser_context_parse_string(GSDLParserContext *self, const char *str) {
	GError *err = NULL;
	self->tokenizer = gsdl_tokenizer_new_from_string(str, &err);

	if (!self->tokenizer) {
		MAYBE_CALLBACK(self->parser->error, self, err, self->user_data);
		return false;
	}

	return _parse(self);
}
