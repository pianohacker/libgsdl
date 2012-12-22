#include <glib.h>
#include <parser.h>
#include <syntax.h>

void start_tag_appender(
		GSDLParserContext *context,
		const gchar *name,
		GValue* const *values,
		gchar* const *attr_names,
		GValue* const *attr_values,
		gpointer user_data,
		GError **err
	) {

	GString *result = (GString*) user_data;
	GString *repr = g_string_new("START: ");

	g_string_append(repr, name);

	g_string_append_c(repr, '\n');

	g_string_append(result, repr->str);
}


void end_tag_appender(
		GSDLParserContext *context,
		const char *name,
		gpointer user_data,
		GError **err
	) {

	GString *result = (GString*) user_data;
	GString *repr = g_string_new("END: ");

	g_string_append(repr, name);

	g_string_append_c(repr, '\n');

	g_string_append(result, repr->str);
}


void error_appender(
		GSDLParserContext *context,
		GError *err,
		gpointer user_data
	) {

	GString *result = (GString*) user_data;

	g_string_append(result, "ERROR: ");
	g_string_append(result, err->message);
}

GSDLParser appender_parser = {
	start_tag_appender,
	end_tag_appender,
	error_appender
};

//> Actual Tests
void test_parser_identifier_only() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "tag");
	g_assert_cmpstr(result->str, ==, "START: tag\nEND: tag\n");
	g_assert(success);
}

void test_parser_identifier_nested() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "outer { inner }");
	g_assert_cmpstr(result->str, ==, "START: outer\nSTART: inner\nEND: inner\nEND: outer\n");
	g_assert(success);
}

int main(int argc, char **argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/parser/identifier_only", test_parser_identifier_only);
	g_test_add_func("/parser/identifier_nested", test_parser_identifier_nested);

	return g_test_run();
}
