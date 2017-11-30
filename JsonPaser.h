#ifndef MY_JSON_PASER_JSONPASER_H
#define MY_JSON_PASER_JSONPASER_H
#include<string>
#include<vector>
#include<map>
#include<memory>

enum {
    PARSE_OK = 0,
    PARSE_EXPECT_VALUE,
    PARSE_INVALID_VALUE,
    PARSE_ROOT_NOT_SINGULAR,
    PARSE_NUMBER_TOO_BIG,
    PARSE_INVALID_STRING_ESCAPE,
    PARSE_MISS_QUOTATION_MARK,
    PARSE_INVALID_STRING_CHAR,
    PARSE_INVALID_UNICODE_HEX,
    PARSE_INVALID_UNICODE_SURROGATE,
    PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    PARSE_MISS_KEY,
    PARSE_MISS_COLON,
    PARSE_MISS_COMMA_OR_CURLY_BRACKET,
    STRINGIFY_OK
};// some return status of the functions

class document;
class Value
{
    friend class document;
private:
    typedef enum {
        NUll,
        FALSE,
        TRUE,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT } json_type;
    double n = 0.0;
    bool b = false;
    std::string s;
    std::vector<std::shared_ptr<Value>> array;
    std::map<std::string,std::shared_ptr<Value>> object;
    json_type type=NUll;
public:
    bool IsNull() const
    {
        return (NUll == type );
    }

    bool IsBool() const
    {
        return (type == FALSE || type == TRUE);
    }

    bool IsNumber() const
    {
        return (type == NUMBER);
    }

    bool IsString() const
    {
        return (type == STRING);
    }

    bool IsArray() const
    {
        return (type == ARRAY);
    }

    bool IsObject() const
    {
        return (type == OBJECT);
    }

    const std::string& GetString() const
    {
        return s;
    }

    std::string& GetString()
    {
        return s;
    }

    const double& GetNumber() const
    {
        return n;
    }

    double& GetNumber()
    {
        return n;
    }

    const bool& GetBool() const
    {
        return b;
    }

    bool& GetBool()
    {
        return b;
    }

    const Value& operator[] (const std::string &a) const
    {
        return *((*object.find(a)).second);
    }

    Value& operator[] (const std::string &a)
    {
        return *object[a];
    }

    const Value& operator[] (size_t t) const
    {
        return *array[t];
    }

    Value& operator[] (size_t t)
    {
        return *array[t];
    }

    size_t GetArraysize() const
    {
        return array.size();
    }

    size_t GetObjectsize() const
    {
        return object.size();
    }
};

class document
{
public:
    int parse(const std::string& json);
    bool IsNull() const
    {
        return (jsonvalue.type == Value::NUll );
    }

    bool IsBool() const
    {
        return (jsonvalue.type == Value::FALSE || jsonvalue.type == Value::TRUE);
    }

    bool IsNumber() const
    {
        return (jsonvalue.type == Value::NUMBER);
    }

    bool IsString() const
    {
        return (jsonvalue.type == Value::STRING);
    }

    bool IsArray() const
    {
        return (jsonvalue.type == Value::ARRAY);
    }

    bool IsObject() const
    {
        return (jsonvalue.type == Value::OBJECT);
    }

    const std::string& GetString() const
    {
        return jsonvalue.s;
    }

    std::string& GetString()
    {
        return jsonvalue.s;
    }

    const double& GetNumber() const
    {
        return jsonvalue.n;
    }

    double& GetNumber()
    {
        return jsonvalue.n;
    }

    const bool& GetBool() const
    {
        return jsonvalue.b;
    }

    bool& GetBool()
    {
        return jsonvalue.b;
    }

    size_t GetArraysize() const
    {
        return jsonvalue.array.size();
    }

    size_t GetObjectsize() const
    {
        return jsonvalue.object.size();
    }

    const Value& operator[] (const std::string &a) const
    {
        return *((*jsonvalue.object.find(a)).second);
    }

    Value& operator[] (const std::string &a)
    {
        return *jsonvalue.object[a];
    }

    const Value& operator[] (size_t t) const
    {
        return *jsonvalue.array[t];
    }

    Value& operator[] (size_t t)
    {
        return *jsonvalue.array[t];
    }
private:
    std::string::iterator current; //the current position of the iterator when in parse
    Value jsonvalue;
    std::string json_doc;
    int parse_value();
    int parse_whitespace();
    int parse_null();
    int parse_false();
    int parse_true();
    int parse_numble();
    int parse_string();
    int parse_array();
    int parse_object();
    bool parse_hex4(unsigned& u);
    void encode_utf8(const unsigned& u);

};

#endif
