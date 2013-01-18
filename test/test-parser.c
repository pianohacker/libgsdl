#include <glib.h>
#include <parser.h>
#include <syntax.h>
#include <unistd.h>

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
		g_string_append_c(repr, '=');
		g_string_append(repr, G_VALUE_TYPE_NAME(*attr_values));
		g_string_append_c(repr, ':');
		g_string_append(repr, g_strdup_value_contents(*attr_values));
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
	bool success = gsdl_parser_context_parse_string(context, "tag 58 -5 2L -32L 69.2 -52.3 43.2D 25.3f -92.432f 53.25BD -8923.33bd");
	g_assert_cmpstr(result->str, ==, "(tag,gint:58,gint:-5,gint64:2,gint64:-32,gdouble:69.200000,gdouble:-52.300000,gdouble:43.200000,gfloat:25.299999,gfloat:-92.431999,gsdldecimal:53.25,gsdldecimal:-8923.33\ntag)\n");
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

void test_parser_value_datetime() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "tag 2042/4/20 2012/2/5 5:30 \"c\" 1924/11/4 19:34:5 \"b\" 2001/02/23 4:00:23.52 2040/2/3 4:00-America/Denver 502/10/10 12:00:00-GMT+4:15");
	g_assert_cmpstr(result->str, ==, "(tag,gsdldate:2042-04-20,gsdldatetime:2012-02-05T05:30:00-0700,gchararray:\"c\",gsdldatetime:1924-11-04T19:34:05-0700,gchararray:\"b\",gsdldatetime:2001-02-23T04:00:23.520-0700,gsdldatetime:2040-02-03T04:00:00-0700,gsdldatetime:502-10-10T12:00:00+0415\ntag)\n");
	g_assert(success);
}

void test_parser_value_timespan() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "tag 00:40:20 42:00:52 30d:00:1:20 -50d:32:23:21 20:42:32.324 -323:00:00.342");
	g_assert_cmpstr(result->str, ==, "(tag,gsdltimespan:2420000000,gsdltimespan:151252000000,gsdltimespan:2592080000000,gsdltimespan:-4436601000000,gsdltimespan:74552324000,gsdltimespan:-1162800342000\ntag)\n");
	g_assert(success);
}

void test_parser_value_binary() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "tag [YmluYXJ5] [cGFkZGVkI}GJpbmFyeQ==] [ZW1iZWRkZWQAbnVsbHM=]");
	g_assert_cmpstr(result->str, ==, "(tag,gsdlbinary:binary,gsdlbinary:padded binary,gsdlbinary:embedded\\\\0nulls\ntag)\n");
	g_assert(success);
}

void test_parser_value_char() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "tag '\xe2\x80\x93'");
	g_assert_cmpstr(result->str, ==, "(tag,gsdlunichar:\\342\\200\\223\ntag)\n");
	g_assert(success);
}

void test_parser_attr_full() {
	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_string(context, "[ZW1iZWRkZWQAbnVsbHM=] int=58 long=-32L double=52.3 float=25.3f big=-8923.33bd bool=true nil=null str=\"abc\" date=2042/4/20 datetime=2012/2/5 5:30 timespan=42:00:52 timespan2=20:42:32.324 bin=[YmluYXJ5]");
	g_assert_cmpstr(result->str, ==, "(content,gsdlbinary:embedded\\\\0nulls,int=gint:58,long=gint64:-32,double=gdouble:52.300000,float=gfloat:25.299999,big=gsdldecimal:-8923.33,bool=gboolean:TRUE,nil=gpointer:NULL,str=gchararray:\"abc\",date=gsdldate:2042-04-20,datetime=gsdldatetime:2012-02-05T05:30:00-0700,timespan=gsdltimespan:151252000000,timespan2=gsdltimespan:74552324000,bin=gsdlbinary:binary\ncontent)\n");
	g_assert(success);
}

void test_parser_file_full() {
	char *filename;
	GIOChannel *channel = g_io_channel_unix_new(g_file_open_tmp("test-tokenizer.XXXXXX", &filename, NULL));
	g_io_channel_write_unichar(channel, 0xe9, NULL);
	g_io_channel_write_chars(channel, "toile \"strings have \\\"", -1, NULL, NULL);
	g_io_channel_write_unichar(channel, 0x2032, NULL);
	g_io_channel_write_chars(channel, "s\" '", -1, NULL, NULL);
	g_io_channel_write_unichar(channel, 0x2013, NULL);
	g_io_channel_write_chars(channel, "' 123 45L; 56.79f 1.2D\\\n3.4BD true\n2012/12/19 0d:00:01:03.123 [abcs21+==] null", -1, NULL, NULL);
	g_io_channel_shutdown(channel, true, NULL);

	GString *result = g_string_new("");
	GSDLParserContext *context = gsdl_parser_context_new(&appender_parser, (gpointer) result);

	g_assert(context != NULL);
	bool success = gsdl_parser_context_parse_file(context, filename);
	g_assert_cmpstr(result->str, ==, "(\xc3\xa9toile,gchararray:\"strings have \\\"\\342\\200\\262s\",gsdlunichar:\\342\\200\\223,gint:123,gint64:45\n\xc3\xa9toile)\n(content,gfloat:56.790001,gdouble:1.200000,gsdldecimal:3.4,gboolean:TRUE\ncontent)\n(content,gsdldate:2012-12-19,gsdltimespan:63123000,gsdlbinary:i\\xb7,\\xdb_,gpointer:NULL\ncontent)");
	g_assert(success);
}

#define TEST(name) g_test_add_func("/parser/"#name, test_parser_##name)

int main(int argc, char **argv) {
	g_type_init();
	g_test_init(&argc, &argv, NULL);

	TEST(identifier_only);
	TEST(identifier_nested);
	TEST(identifier_sequence);
	TEST(value_numbers);
	TEST(value_keywords);
	TEST(value_strings);
	TEST(value_datetime);
	TEST(value_timespan);
	TEST(value_binary);
	TEST(value_char);
	TEST(attr_full);
	TEST(file_full);

	return g_test_run();
}
