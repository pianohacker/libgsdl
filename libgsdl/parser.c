#include "parser.h"
#include "tokenizer.h"

struct _GSDLParserContext {
	GSDLTokenizer *tokenizer;

	GSDLParser *parser;
	gpointer user_data;

	GSList *parser_stack;
	GSList *data_stack;
};

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
