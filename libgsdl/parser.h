#ifndef __PARSER_H__
#define __PARSER_H__

#include <glib.h>
#include <stdbool.h>

typedef struct

typedef struct {
	void (*start_tag)(
		GSDLParserContext *context,
		const char *name,
		const char **attr_names,
		const GValue **attr_values,
		const GValue **values,
		gpointer user_data,
		GError **error,
	);

	void (*end_tag)(
		GSDLParserContext *context,
		const char *name,
		gpointer user_data,
		GError **error,
	);

	void (*error)(
		GSDLParserContext *context,
		GError *error,
		gpointer user_data;
	);

} GSDLParser;

typedef struct _GSDLParserContext GSDLParserContext;

extern GSDLParserContext* gsdl_parser_context_new(GSDLParser *parser, gpointer user_data);

extern void gsdl_parser_context_push(GSDLParserContext *self, GSDLParser *parser, gpointer user_data);
extern gpointer gsdl_parser_context_pop(GSDLParserContext *self);

extern bool gsdl_parser_context_parse_file(const char *filename);
extern bool gsdl_parser_context_parse_string(const char *str);

#endif
