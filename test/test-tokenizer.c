#include <glib.h>
#include <tokenizer.h>

#define ASSERT_TOKEN(t) do { bool success = gsdl_tokenizer_next(tokenizer, &token, &error); g_assert_no_error(error); g_assert(success); g_assert_cmpint(token->type, ==, t); } while(0)
#define ASSERT_TOKEN_VAL(t, v) do { bool success = gsdl_tokenizer_next(tokenizer, &token, &error); g_assert_no_error(error); g_assert(success); g_assert_cmpint(token->type, ==, t); g_assert_cmpstr(token->val, ==, v); } while(0)

void test_tokenizer_string_simple() {
	GError *error = NULL;
	GSDLTokenizer *tokenizer = gsdl_tokenizer_new_from_string("tag val=2", &error);

	g_assert_no_error(error);
	g_assert(tokenizer != NULL);

	GSDLToken *token;
	ASSERT_TOKEN_VAL(T_IDENTIFIER, "tag");
	ASSERT_TOKEN_VAL(T_IDENTIFIER, "val");
	ASSERT_TOKEN('=');
	ASSERT_TOKEN_VAL(T_NUMBER, "2");
	ASSERT_TOKEN(T_EOF);
	g_assert(!gsdl_tokenizer_next(tokenizer, &token, &error));
}

void test_tokenizer_string_comments() {
	GError *error = NULL;
	GSDLTokenizer *tokenizer = gsdl_tokenizer_new_from_string("tag -- Comment", &error);

	g_assert_no_error(error);
	g_assert(tokenizer != NULL);

	GSDLToken *token;
	ASSERT_TOKEN_VAL(T_IDENTIFIER, "tag");
	ASSERT_TOKEN(T_EOF);
	g_assert(!gsdl_tokenizer_next(tokenizer, &token, &error));

	tokenizer = gsdl_tokenizer_new_from_string("2/3 // Comment\ntag # Comentario\n52 - 242 -- Comment on the comment", &error);

	g_assert_no_error(error);
	g_assert(tokenizer != NULL);

	ASSERT_TOKEN_VAL(T_NUMBER, "2");
	ASSERT_TOKEN('/');
	ASSERT_TOKEN_VAL(T_NUMBER, "3");
	ASSERT_TOKEN('\n');
	ASSERT_TOKEN_VAL(T_IDENTIFIER, "tag");
	ASSERT_TOKEN('\n');
	ASSERT_TOKEN_VAL(T_NUMBER, "52");
	ASSERT_TOKEN('-');
	ASSERT_TOKEN_VAL(T_NUMBER, "242");
	ASSERT_TOKEN(T_EOF);
	g_assert(!gsdl_tokenizer_next(tokenizer, &token, &error));
}

void test_tokenizer_string_numbers() {
	GError *error = NULL;
	GSDLTokenizer *tokenizer = gsdl_tokenizer_new_from_string("123L 123.43f 52.5D 352.12BD", &error);

	g_assert_no_error(error);
	g_assert(tokenizer != NULL);

	GSDLToken *token;
	ASSERT_TOKEN_VAL(T_LONGINTEGER, "123");
	ASSERT_TOKEN_VAL(T_NUMBER, "123");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_FLOAT_END, "43");
	ASSERT_TOKEN_VAL(T_NUMBER, "52");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_D_NUMBER, "5");
	ASSERT_TOKEN_VAL(T_NUMBER, "352");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_DECIMAL_END, "12");
	ASSERT_TOKEN(T_EOF);
	g_assert(!gsdl_tokenizer_next(tokenizer, &token, &error));
}

void test_tokenizer_string_strings() {
	GError *error = NULL;
	GSDLTokenizer *tokenizer = gsdl_tokenizer_new_from_string("'c' '\\'' \"simple\" \"escapes\\t\\\"\" \"multiple \\\n     lines\" `backquote  \r\n  st\\ring`", &error);

	g_assert_no_error(error);
	g_assert(tokenizer != NULL);

	GSDLToken *token;
	ASSERT_TOKEN_VAL(T_CHAR, "c");
	ASSERT_TOKEN_VAL(T_CHAR, "'");
	ASSERT_TOKEN_VAL(T_STRING, "simple");
	ASSERT_TOKEN_VAL(T_STRING, "escapes\t\"");
	ASSERT_TOKEN_VAL(T_STRING, "multiple \nlines");
	ASSERT_TOKEN_VAL(T_STRING, "backquote  \n  st\\ring");
	ASSERT_TOKEN(T_EOF);
	g_assert(!gsdl_tokenizer_next(tokenizer, &token, &error));
}

void test_tokenizer_string_keywords() {
	GError *error = NULL;
	GSDLTokenizer *tokenizer = gsdl_tokenizer_new_from_string("on true ident off nul null false", &error);

	g_assert_no_error(error);
	g_assert(tokenizer != NULL);

	GSDLToken *token;
	ASSERT_TOKEN_VAL(T_BOOLEAN, "on");
	ASSERT_TOKEN_VAL(T_BOOLEAN, "true");
	ASSERT_TOKEN_VAL(T_IDENTIFIER, "ident");
	ASSERT_TOKEN_VAL(T_BOOLEAN, "off");
	ASSERT_TOKEN_VAL(T_IDENTIFIER, "nul");
	ASSERT_TOKEN_VAL(T_NULL, "null");
	ASSERT_TOKEN_VAL(T_BOOLEAN, "false");
	ASSERT_TOKEN(T_EOF);
	g_assert(!gsdl_tokenizer_next(tokenizer, &token, &error));
}

void test_tokenizer_string_binary() {
	GError *error = NULL;
	GSDLTokenizer *tokenizer = gsdl_tokenizer_new_from_string("[a32098astas==] [astasr   arstuoth\narstfu]", &error);

	g_assert_no_error(error);
	g_assert(tokenizer != NULL);

	GSDLToken *token;
	ASSERT_TOKEN_VAL(T_BINARY, "a32098astas==");
	ASSERT_TOKEN_VAL(T_BINARY, "astasrarstuotharstfu");
	ASSERT_TOKEN(T_EOF);
	g_assert(!gsdl_tokenizer_next(tokenizer, &token, &error));
}

void test_tokenizer_file_full() {
	char *filename;
	GIOChannel *channel = g_io_channel_unix_new(g_file_open_tmp("test-tokenizer.XXXXXX", &filename, NULL));
	g_io_channel_write_unichar(channel, 0xe9, NULL);
	g_io_channel_write_chars(channel, "toile \"strings have \\\"", -1, NULL, NULL);
	g_io_channel_write_unichar(channel, 0x2032, NULL);
	g_io_channel_write_chars(channel, "s\" '", -1, NULL, NULL);
	g_io_channel_write_unichar(channel, 0x2013, NULL);
	g_io_channel_write_chars(channel, "' 123 45L 56.79f 1.2D\\\n3.4BD true\n2012/12/19 0d:00:01:03.123 [abcs21+] null", -1, NULL, NULL);
	g_io_channel_shutdown(channel, true, NULL);

	GError *error = NULL;
	GSDLTokenizer *tokenizer = gsdl_tokenizer_new(filename, &error);

	g_assert_no_error(error);
	g_assert(tokenizer != NULL);

	GSDLToken *token;
	ASSERT_TOKEN_VAL(T_IDENTIFIER, "\xc3\xa9toile");
	ASSERT_TOKEN_VAL(T_STRING, "strings have \"\xe2\x80\xb2s");
	ASSERT_TOKEN_VAL(T_CHAR, "\xe2\x80\x93");
	ASSERT_TOKEN_VAL(T_NUMBER, "123");
	ASSERT_TOKEN_VAL(T_LONGINTEGER, "45");
	ASSERT_TOKEN_VAL(T_NUMBER, "56");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_FLOAT_END, "79");
	ASSERT_TOKEN_VAL(T_NUMBER, "1");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_D_NUMBER, "2");
	ASSERT_TOKEN_VAL(T_NUMBER, "3");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_DECIMAL_END, "4");
	ASSERT_TOKEN_VAL(T_BOOLEAN, "true");
	ASSERT_TOKEN('\n');
	ASSERT_TOKEN_VAL(T_NUMBER, "2012");
	ASSERT_TOKEN('/');
	ASSERT_TOKEN_VAL(T_NUMBER, "12");
	ASSERT_TOKEN('/');
	ASSERT_TOKEN_VAL(T_NUMBER, "19");
	ASSERT_TOKEN_VAL(T_D_NUMBER, "0");
	ASSERT_TOKEN(':');
	ASSERT_TOKEN_VAL(T_NUMBER, "00");
	ASSERT_TOKEN(':');
	ASSERT_TOKEN_VAL(T_NUMBER, "01");
	ASSERT_TOKEN(':');
	ASSERT_TOKEN_VAL(T_NUMBER, "03");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_NUMBER, "123");
	ASSERT_TOKEN_VAL(T_BINARY, "abcs21+");
	ASSERT_TOKEN_VAL(T_NULL, "null");
	ASSERT_TOKEN(T_EOF);
	g_assert(!gsdl_tokenizer_next(tokenizer, &token, &error));

	g_unlink(filename);
}

void test_tokenizer_string_invalid_utf8() {
	GError *error = NULL;
	GSDLTokenizer *tokenizer = gsdl_tokenizer_new_from_string("\xff", &error);

	g_assert(tokenizer == NULL);
	g_assert_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE);
}

int main(int argc, char **argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/tokenizer/string_simple", test_tokenizer_string_simple);

	g_test_add_func("/tokenizer/string_binary", test_tokenizer_string_binary);
	g_test_add_func("/tokenizer/string_comments", test_tokenizer_string_comments);
	g_test_add_func("/tokenizer/string_keywords", test_tokenizer_string_keywords);
	g_test_add_func("/tokenizer/string_numbers", test_tokenizer_string_numbers);
	g_test_add_func("/tokenizer/string_strings", test_tokenizer_string_strings);

	g_test_add_func("/tokenizer/file_full", test_tokenizer_file_full);

	g_test_add_func("/tokenizer/string_invalid_utf8", test_tokenizer_string_invalid_utf8);

	return g_test_run();
}
