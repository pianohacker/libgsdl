#include <glib.h>
#include <tokenizer.h>

#define ASSERT_TOKEN(t) g_assert(gsdl_tokenizer_next(tokenizer, &token, &error)); g_assert_no_error(error); g_assert_cmpint(token->type, ==, t)
#define ASSERT_TOKEN_VAL(t, v) g_assert(gsdl_tokenizer_next(tokenizer, &token, &error)); g_assert_no_error(error); g_assert_cmpint(token->type, ==, t); g_assert_cmpstr(token->val, ==, v)

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
	ASSERT_TOKEN_VAL(T_LONGINTEGER, "2");
	ASSERT_TOKEN_VAL(T_NUMBER, "123");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_FLOAT_END, "43");
	ASSERT_TOKEN_VAL(T_NUMBER, "52");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_D_NUMBER, "5");
	ASSERT_TOKEN_VAL(T_NUMBER, "351");
	ASSERT_TOKEN('.');
	ASSERT_TOKEN_VAL(T_DECIMAL_END, "12");
	ASSERT_TOKEN(T_EOF);
	g_assert(!gsdl_tokenizer_next(tokenizer, &token, &error));
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
	g_test_add_func("/tokenizer/string_comments", test_tokenizer_string_comments);
	g_test_add_func("/tokenizer/string_numbers", test_tokenizer_string_numbers);
	g_test_add_func("/tokenizer/string_invalid_utf8", test_tokenizer_string_invalid_utf8);

	return g_test_run();
}
