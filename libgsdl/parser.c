#include "parser.h"
#include "syntax.h"
#include "tokenizer.h"

struct _GSDLParserContext {
	GSDLTokenizer *cur_tokenizer;
	GSDLToken *peek_token;

	GSDLParser *parser;
	gpointer user_data;

	GSList *parser_stack;
	GSList *data_stack;
};

#define MAYBE_CALLBACK(callback, ...) if (callback) callback(__VA_ARGS___)
#define REQUIRE(expr) if (!expr) return false;

GSDLParserContext* gsdl_parser_context_new(GSDLParser *parser, gpointer user_data) {
	GSDLParserContext *self = g_new0(GSDLParserContext);

	self->parser = parser;
	self->user_data = user_data;
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
	if (self->peek_token)
		GSDLToken *result = self->peek_token;
		self->peek_token = NULL;
		*token = self->peek_token;

		return true;
	} else {
		GError *error;

		if (!gsdl_tokenizer_next(self->tokenizer, token, &error)) {
			MAYBE_CALLBACK(self->parser->error, self, error, self->user_data);
			return false;
		} else {
			return true;
		}
	}
}

static bool _peek(GSDLParserContext *self, GSDLToken **token) {
	if (self->peek_token)
		*token = self->peek_token;

		return true;
	} else {
		GError *error;

		if (!gsdl_tokenizer_next(self->tokenizer, &(self->peek_token), &error)) {
			MAYBE_CALLBACK(self->parser->error, self, error, self->user_data);
			return false;
		} else {
			*token = self->peek_token;
			return true;
		}
	}
}

static bool _consume(GSDLParserContext *self) {
	g_assert(self->peek_token != NULL);

	self->peek_token = NULL;
}

static void _error(GSDLTokenizer *self, GSDLToken *token, GSDLSyntaxError err_type, char *msg) {
	GError *err;
	g_set_error(&err,
		GSDL_SYNTAX_ERROR,
		err_type,
		"%s in %s, line %d, column %d",
		msg,
		gsdl_tokenizer_get_filename(self->tokenizer),
		token->line,
		token->col,
	);
	MAYBE_CALLBACK(self->parser->error, self, err, self->user_data);
}

//> Parser Functions
static bool _parse(GSDLParserContext *self) {
	GSDLToken *token;
	for (;;) {
		REQUIRE(_peek(self, &token));

		if (token->type == T_EOF) {
			break;
		} else {
			_parse_tag(self);
		}
	}
}

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

static bool _parse_tag(GSDLParserContext *self) {
	GSDLToken *first, *token;
	char *name = g_strdup("contents");

	GArray *values = g_array_new(TRUE, FALSE, sizeof(GValue));

	GArray *attr_names = g_array_new(TRUE, FALSE, sizeof(char*));
	GArray *attr_values = g_array_new(TRUE, FALSE, sizeof(GValue));

	REQUIRE(_peek(self, &first));

	if (first->type == T_IDENTIFIER) {
		_consume(self);

		REQUIRE(_peek(self, &token));

		if (token->type == '=') {
			_error(
				self,
				first,
				"At least one value required for an anonymous tag",
			);

			return false;
		}

		g_free(name);
		name = g_strdup(first->val);
		gsdl_token_free(first);
	}

	bool peek_success = true;

	while ((_peek(self, &token) || (peek_success = false)) && _token_is_value(token)) {
		GValue *value = g_slice_new(GValue);
		REQUIRE(_parse_value(self, value));
		g_array_append_val(values, value);
	}
	REQUIRE(peek_success);
}

bool gsdl_parser_context_parse_file(GSDLParserContext *self, const char *filename) {
	GError *err;
	self->tokenizer = gsdl_tokenizer_new(filename, &err);

	if (!self->tokenizer) {
		MAYBE_CALLBACK(self->parser->error, self, err, self->user_data);
		return false;
	}

	return _parse_tag(self);
}

bool gsdl_parser_context_parse_string(GSDLParserContext *self, const char *str) {
	GError *err;
	self->tokenizer = gsdl_tokenizer_new_from_string(str, &err);

	if (!self->tokenizer) {
		MAYBE_CALLBACK(self->parser->error, self, err, self->user_data);
		return false;
	}

	return _parse(self);
}
