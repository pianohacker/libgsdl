#include <glib.h>
#include <tokenizer.h>

#define ASSERT_TOKEN(t) g_assert(gsdl_tokenizer_next(tokenizer, &token, &error)); g_assert_no_error(error); g_assert_cmpint(token->type, ==, t)
#define ASSERT_TOKEN_VAL(t, v) g_assert(gsdl_tokenizer_next(tokenizer, &token, &error)); g_assert_no_error(error); g_assert_cmpint(token->type, ==, t); g_assert_cmpstr(token->val, ==, v)

void test_tokenizer_string() {
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

	tokenizer = gsdl_tokenizer_new_from_string("\xff", &error);
	g_assert_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE);
}

int main(int argc, char **argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/tokenizer/string", test_tokenizer_string);

	return g_test_run();
}
