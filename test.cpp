#include<iostream>
#include "JsonPaser.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ(expect, actual) \
    do {\
        test_count++;\
        if ((expect) == (actual))\
            test_pass++;\
        else {\
            std::cerr<<__FILE__<<":"<<__LINE__<<": expect:"<<expect<<" actual: "<<actual<<std::endl;\
            main_ret = 1;\
        }\
    } while(0)

static void test_parse_null() {
    document a;
    EXPECT_EQ(PARSE_OK, a.parse("null"));
    EXPECT_EQ(true,a.IsNull());
}

static void test_parse_true() {
    document a;
    EXPECT_EQ(PARSE_OK, a.parse("true"));
    EXPECT_EQ(true, a.IsBool());
    EXPECT_EQ(true,a.GetBool());
}

static void test_parse_false() {
    document a;
    EXPECT_EQ(PARSE_OK, a.parse("false"));
    EXPECT_EQ(true, a.IsBool());
    EXPECT_EQ(false,a.GetBool());
}

#define TEST_NUMBER(expect, json)\
    do {\
        auto a = new document;\
        EXPECT_EQ(PARSE_OK, a->parse(json));\
        EXPECT_EQ(true, a->IsNumber());\
        EXPECT_EQ(expect, a->GetNumber());\
        delete a;\
    } while(0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
    do {\
        auto a = new document;\
        EXPECT_EQ(PARSE_OK, a->parse(json));\
        EXPECT_EQ(true, a->IsString());\
        EXPECT_EQ(expect,a->GetString());\
        delete a;\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    //TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

#define TEST_ERROR(error, json)\
    do {\
        auto a = new document;\
        EXPECT_EQ(error, a->parse(json));\
        delete a;\
    } while(0)

static void test_parse_expect_value() {
    TEST_ERROR(PARSE_EXPECT_VALUE, "");
    TEST_ERROR(PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(PARSE_INVALID_VALUE, "?");

    /* invalid number */
    TEST_ERROR(PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "null x");

    /* invalid number */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_array() {
    document a;
    EXPECT_EQ(PARSE_OK, a.parse("[ ]"));
    EXPECT_EQ(true, a.IsArray());
    EXPECT_EQ(0, a.GetArraysize());

    document b;
    EXPECT_EQ(PARSE_OK, b.parse("[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ(true, b.IsArray());
    EXPECT_EQ(5, b.GetArraysize());
    EXPECT_EQ(true, b[0].IsNull());
    EXPECT_EQ(true, b[1].IsBool());
    EXPECT_EQ(true, b[2].IsBool());
    EXPECT_EQ(true, b[3].IsNumber());
    EXPECT_EQ(true, b[4].IsString());
    EXPECT_EQ(123.0, b[3].GetNumber());
    EXPECT_EQ("abc", b[4].GetString());


    document c;
    EXPECT_EQ(PARSE_OK, c.parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ(true, c.IsArray());
    EXPECT_EQ(4, c.GetArraysize());
    for (size_t i = 0; i < 4; i++) {
        EXPECT_EQ(true, c[i].IsArray());
        EXPECT_EQ(i, c[i].GetArraysize());
        for (size_t j = 0; j < i; j++) {
            EXPECT_EQ(true, c[i][j].IsNumber());
            EXPECT_EQ((double) j, c[i][j].GetNumber());
        }
    }
}

static void test_parse_miss_comma_or_square_bracket() {
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_object() {
    document a;
    EXPECT_EQ(PARSE_OK, a.parse(" { } "));
    EXPECT_EQ(true, a.IsObject());
    EXPECT_EQ(0, a.GetObjectsize());

    document b;
    EXPECT_EQ(PARSE_OK, b.parse(
                                            " { "
                                                    "\"n\" : null , "
                                                    "\"f\" : false , "
                                                    "\"t\" : true , "
                                                    "\"i\" : 123 , "
                                                    "\"s\" : \"abc\", "
                                                    "\"a\" : [ 1, 2, 3 ],"
                                                    "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                                    " } "
    ));
    EXPECT_EQ(true, b.IsObject());
    EXPECT_EQ(7, b.GetObjectsize());
    EXPECT_EQ(true,b["n"].IsNull() );
    EXPECT_EQ(false,b["f"].GetBool());
    EXPECT_EQ(true,b["t"].GetBool());
    EXPECT_EQ(double (123),b["i"].GetNumber());
    EXPECT_EQ("abc",b["s"].GetString() );
    EXPECT_EQ(true, b["a"].IsArray());
    for (size_t i = 0; i < 3; i++) {
        EXPECT_EQ(i + 1.0, b["a"][i].GetNumber());
    }
    EXPECT_EQ(true, b["o"].IsObject());
    for (size_t i = 0; i < 3; i++)
    {
        char a ='1'+i;
        std::string s;
        s.push_back(a);
        EXPECT_EQ(1.0+i,b["o"][s].GetNumber());
    }
}

static void test_parse_miss_key() {
    TEST_ERROR(PARSE_MISS_KEY, "{:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{1:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{true:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{false:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{null:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{[]:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{{}:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
    TEST_ERROR(PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_string();
    test_parse_array();
    test_parse_miss_comma_or_square_bracket;
    test_parse_object();
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
}

int main() {
    test_parse();
    std::cout<<test_pass<<"/"<<test_count<<" "<<test_pass * 100.0 / test_count<<"%passed"<<std::endl;
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}