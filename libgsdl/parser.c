#include "parser.h"
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

static bool _parse_tag(GSDLParserContext *self) {
	GSDLToken *first;
	char *name = "contents";

	REQUIRE(_read(self, &first));
}

bool gsdl_parser_context_parse_file(GSDLParserContext *self, const char *filename) {
	GError *error;
	self->tokenizer = gsdl_tokenizer_new(filename, &error);

	if (!self->tokenizer) {
		MAYBE_CALLBACK(self->parser->error, self, error, self->user_data);
		return false;
	}

	return _parse_tag(self);
}

bool gsdl_parser_context_parse_string(GSDLParserContext *self, const char *str) {
	GError *error;
	self->tokenizer = gsdl_tokenizer_new_from_string(str, &error);

	if (!self->tokenizer) {
		MAYBE_CALLBACK(self->parser->error, self, error, self->user_data);
		return false;
	}

	return _parse(self);
}
