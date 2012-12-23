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
	GString *repr = g_string_new("(");

	g_string_append(repr, name);

	for (; *values; values++) {
		g_string_append_c(repr, ',');
		g_string_append(repr, G_VALUE_TYPE_NAME(*values));
		g_string_append_c(repr, ':');
		g_string_append(repr, g_strdup_value_contents(*values));
	}

	for (; *attr_names; attr_names++, attr_values++) {
		g_string_append_c(repr, ',');
		g_string_append(repr, *attr_names);
		g_string_append(repr, G_VALUE_TYPE_NAME(*attr_values));
	}

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
	GString *repr = g_string_new("");

	g_string_append(repr, name);

	g_string_append(repr, ")\n");

	g_string_append(result, repr->str);
}


void error_appender(
		GSDLParserContext *context,
		GError *err,
		gpointer user_data
	) {

	GString *result = (GString*) user_data;

	g_string_append(result, "E: ");
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
	g_assert_cmpstr(result->str, ==, "(tag\ntag)\n");
	g_assert(success);
}

void test_parser_identifier_nested() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);
	g_assert(context != NULL);

	bool success = gsdl_parser_context_parse_string(context, "outer { inner }");
	g_assert_cmpstr(result->str, ==, "(outer\n(inner\ninner)\nouter)\n");
	g_assert(success);

	g_string_truncate(result, 0);
	success = gsdl_parser_context_parse_string(context, "first { second { third } }");
	g_assert_cmpstr(result->str, ==, "(first\n(second\n(third\nthird)\nsecond)\nfirst)\n");
	g_assert(success);
}

void test_parser_identifier_sequence() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);
	g_assert(context != NULL);

	bool success = gsdl_parser_context_parse_string(context, "one; two\n three\n\nfour");
	g_assert_cmpstr(result->str, ==, "(one\none)\n(two\ntwo)\n(three\nthree)\n(four\nfour)\n");
	g_assert(success);

	g_string_truncate(result, 0);
	success = gsdl_parser_context_parse_string(context, "first { second; third\n }");
	g_assert_cmpstr(result->str, ==, "(first\n(second\nsecond)\n(third\nthird)\nfirst)\n");
	g_assert(success);
}

void test_parser_value_numbers() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "tag 58 2L");
	g_assert_cmpstr(result->str, ==, "(tag,gint:58,gint64:2\ntag)\n");
	g_assert(success);
}

void test_parser_value_keywords() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "tag true false null off on");
	g_assert_cmpstr(result->str, ==, "(tag,gboolean:TRUE,gboolean:FALSE,gpointer:NULL,gboolean:FALSE,gboolean:TRUE\ntag)\n");
	g_assert(success);
}

void test_parser_value_strings() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "tag \"abc\"");
	g_assert_cmpstr(result->str, ==, "(tag,gchararray:\"abc\"\ntag)\n");
	g_assert(success);
}

int main(int argc, char **argv) {
	g_type_init();
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/parser/identifier_only", test_parser_identifier_only);
	g_test_add_func("/parser/identifier_nested", test_parser_identifier_nested);
	g_test_add_func("/parser/identifier_sequence", test_parser_identifier_sequence);
	g_test_add_func("/parser/value_numbers", test_parser_value_numbers);
	g_test_add_func("/parser/value_keywords", test_parser_value_keywords);
	g_test_add_func("/parser/value_strings", test_parser_value_strings);

	return g_test_run();
}
