#ifndef __PARSER_H__
#define __PARSER_H__

#include <glib.h>
#include <glib-object.h>
#include <stdbool.h>

typedef struct _GSDLParserContext GSDLParserContext;

typedef struct {
	void (*start_tag)(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	);

	void (*end_tag)(
		GSDLParserContext *context,
		const gchar *name,
		gpointer user_data,
		GError **errr
	);

	void (*error)(
		GSDLParserContext *context,
		GError *errr,
		gpointer user_data
	);

} GSDLParser;

extern GSDLParserContext* gsdl_parser_context_new(GSDLParser *parser, gpointer user_data);

extern void gsdl_parser_context_push(GSDLParserContext *self, GSDLParser *parser, gpointer user_data);
extern gpointer gsdl_parser_context_pop(GSDLParserContext *self);

extern bool gsdl_parser_context_parse_file(GSDLParserContext *self, const char *filename);
extern bool gsdl_parser_context_parse_string(GSDLParserContext *self, const char *str);

#endif
