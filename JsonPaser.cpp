#include <string>
#include "JsonPaser.h"
#include <cmath>
#include<memory>
#include <map>
#include <cstdlib>
#include <cerrno>


int document::parse_whitespace()
{
    while (*current == ' ' || *current == '\t' || *current == '\n' || *current == '\r')
        ++current;
}

int document::parse_value()
{
    switch (*current)
    {
        case 'n': return parse_null();
        case 'f': return parse_false();
        case 't': return parse_true();
        case '\0': return PARSE_EXPECT_VALUE;
        case '\"': return parse_string();
        case '[': return parse_array();
        case '{': return parse_object();
        default: return parse_numble();
    }
}

int document::parse(const std::string& json)
{
    json_doc = json;
    current = json_doc.begin();
    parse_whitespace();
    int ret;
    if((ret = parse_value()) == PARSE_OK)
    {
        parse_whitespace();
        if(*current != '\0')
        {
            ret = PARSE_ROOT_NOT_SINGULAR;
            jsonvalue.type = Value::NUll;
        }
    }
    return ret;

}

int document::parse_null()
{
    if(*current == 'n')
        ++current;
    if(current[0] != 'u' || current[1] != 'l' || current[2] != 'l')
        return PARSE_INVALID_VALUE;
    current += 3;
    jsonvalue.type = Value::NUll;
    return PARSE_OK;
}

int document::parse_true()
{
    if(*current == 't')
        ++current;
    if(current[0] != 'r' || current[1] != 'u' || current[2] != 'e')
        return PARSE_INVALID_VALUE;
    current += 3;
    jsonvalue.type = Value::TRUE;
    jsonvalue.b = true;
    return PARSE_OK;
}

int document::parse_false()
{
    if(*current == 'f')
        ++current;
    if(current[0] != 'a' || current[1] != 'l' || current[2] != 's' || current[3] != 'e')
        return PARSE_INVALID_VALUE;
    current += 4;
    jsonvalue.type = Value::FALSE;
    jsonvalue.b = false;
    return PARSE_OK;
}

int document::parse_numble()
{
    auto temp = current;
    if(*current == '-')
        ++current;
    if(*current == '0')
        ++current;
    else if(*current >= '1' && *current <= '9')
    {
        ++current;
        while(*current >= '0' && *current <= '9')
            ++current;
    }
    else
        return PARSE_INVALID_VALUE;
    if(*current == '.')
    {
        ++current;
        if(*current >= '0' && *current <= '9')
        {
            ++current;
            while(*current >= '0' && *current <= '9')
                ++current;
        }
        else
            return PARSE_INVALID_VALUE;
    }
    if(*current == 'e' || *current =='E')
    {
        ++current;
        if(*current == '+' || *current == '-')
            ++current;
        if(*current >= '0' && *current <= '9')
        {
            ++current;
            while(*current >= '0' && *current <= '9')
                ++current;
        }
        else
            return PARSE_INVALID_VALUE;
    }
    auto s = json_doc.substr(temp-json_doc.begin());
    jsonvalue.n = strtod(s.c_str(), NULL);
    if (errno == ERANGE && (jsonvalue.n == HUGE_VAL || jsonvalue.n == -HUGE_VAL))
        return PARSE_NUMBER_TOO_BIG;
    jsonvalue.type = Value::NUMBER;
    return PARSE_OK;

}

bool document::parse_hex4(unsigned &u)
{
    for(int i = 0; i<4; ++i)
    {
        if('0' <= *current && *current<= '9'){
            u = u + (*current - '0') * (unsigned)pow(16,3-i);
            ++current;
            continue;
        }
        if('A' <= *current && *current<= 'F'){
            u = u + (*current - 'A'+10) * (unsigned)pow(16,3-i);
            ++current;
            continue;
        }
        if('a' <= *current && *current<= 'f'){
            u = u + (*current - 'a'+10) * (unsigned)pow(16,3-i);
            ++current;
            continue;
        }
        return false;
    }
    return true;
}

void document::encode_utf8(const unsigned &u)
{
    unsigned u1,u2,u3,u4;
    if(0 <= u && u <= 0x7f){
        u1 = (u & 0x7f) | 0X00;
        jsonvalue.s.push_back((char)u1);
    }
    if(0x80 <= u && u <= 0x07ff){
        u1 = ((u >> 6) &0x1f) | 0xc0;
        jsonvalue.s.push_back((char)u1);
        u2 = (u & 0x3f) | 0x80;
        jsonvalue.s.push_back((char)u2);
    }
    if(0x0800 <= u && u <= 0xffff){
        u1 = 0xE0 | ((u >> 12) & 0x0F);
        jsonvalue.s.push_back((char)u1);
        u2 = 0x80 | ((u >>  6) & 0x3F);
        jsonvalue.s.push_back((char)u2);
        u3 = 0x80 | ( u & 0x3F);
        jsonvalue.s.push_back((char)u3);
    }
    if(0x10000 <= u && u <= 0x10FFFF){
        u1 = 0xf0 | ((u >> 18) & 0x07);
        jsonvalue.s.push_back((char)u1);
        u2 = 0x80 | ((u >> 12) & 0x3F);
        jsonvalue.s.push_back((char)u2);
        u3 = 0x80 | ((u >> 6) & 0x3F);
        jsonvalue.s.push_back((char)u3);
        u4 = 0x80 | (u & 0x3F);
        jsonvalue.s.push_back((char)u4);
    }

}

int document::parse_string()
{
    unsigned u = 0;
    unsigned u2 = 0;
    if(*current == '\"')
        ++current;
    for(;;)
    {
        char ch = *current++;
        switch (ch)
        {
            case '\"':
                jsonvalue.type = Value::STRING;
                return PARSE_OK;
            case '\\':
                switch (*current++)
                {
                    case '\"': jsonvalue.s.push_back('\"');break;
                    case '\\': jsonvalue.s.push_back('\\');break;
                    case '/': jsonvalue.s.push_back('/');break;
                    case 'f': jsonvalue.s.push_back('\f');break;
                    case 'n': jsonvalue.s.push_back('\n');break;
                    case 'r': jsonvalue.s.push_back('\r');break;
                    case 't': jsonvalue.s.push_back('\t');break;
                    case 'b': jsonvalue.s.push_back('\b');break;
                    case 'u': if(!parse_hex4(u))
                        {
                            jsonvalue.s.clear();
                            return PARSE_INVALID_UNICODE_HEX;
                        }
                        if(u >= 0xD800 && u <= 0xDBFF)
                        {
                            if(*current++ != '\\')
                            {
                                jsonvalue.s.clear();
                                return PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            if(*current++ != 'u')
                            {
                                jsonvalue.s.clear();
                                return PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            if(!parse_hex4(u2))
                            {
                                jsonvalue.s.clear();
                                return PARSE_INVALID_UNICODE_HEX;
                            }
                            if(u2 < 0xDC00 || u2 > 0xDFFF)
                            {
                                jsonvalue.s.clear();
                                return  PARSE_INVALID_UNICODE_SURROGATE;
                            }
                            u=0x10000 + (u - 0xD800) * 0x400 + (u2 - 0xDC00);
                        }
                        encode_utf8(u);
                        break;
                    default:
                        jsonvalue.s.clear();
                        return PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            case '\0':
                jsonvalue.s.clear();
                return PARSE_MISS_QUOTATION_MARK;
            default:
                if(static_cast<unsigned char>(ch) < 0x20)
                {
                    jsonvalue.s.clear();
                    return PARSE_INVALID_STRING_CHAR;
                }
                jsonvalue.s.push_back(ch);
        }
    }
}

int document::parse_array()
{
    int ret;
    if(*current == '[')
        ++current;
    parse_whitespace();
    if(*current == ']')
    {
        ++current;
        jsonvalue.type = Value::ARRAY;
        return PARSE_OK;
    }
    for(;;)
    {
        document temp;
        temp.json_doc = json_doc.substr(current-json_doc.begin());
        temp.current = temp.json_doc.begin();
        if((ret = temp.parse_value()) != PARSE_OK)
            break;
        current += temp.current - temp.json_doc.begin();
        jsonvalue.array.push_back(std::make_shared<Value>(temp.jsonvalue));
        parse_whitespace();
        if(*current == ',')
        {
            ++current;
            parse_whitespace();
        }
        else if(*current == ']')
        {
            ++current;
            jsonvalue.type=Value::ARRAY;
            return PARSE_OK;
        }
        else
        {
            ret =PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    return ret;
}

int document::parse_object()
{
    int ret;
    if(*current == '{')
        ++current;
    parse_whitespace();
    if(*current == '}')
    {
        ++current;
        jsonvalue.type = Value::OBJECT;
        return PARSE_OK;
    }
    for(;;)
    {
        document temp1;
        temp1.json_doc = json_doc.substr(current-json_doc.begin());
        temp1.current = temp1.json_doc.begin();
        temp1.parse_string();
        if(temp1.jsonvalue.type != Value::STRING)
        {
            ret = PARSE_MISS_KEY;
            break;
        }
        current += temp1.current - temp1.json_doc.begin();
        document temp2;
        parse_whitespace();
        if(*current == ':')
        {
            ++current;
            parse_whitespace();
        }
        else
        {
            ret = PARSE_MISS_COLON;
            break;
        }
        temp2.json_doc = json_doc.substr(current-json_doc.begin());
        temp2.current = temp2.json_doc.begin();
        if((ret = temp2.parse_value()) != PARSE_OK)
            break;
        current += temp2.current - temp2.json_doc.begin();
        auto a = std::make_pair(temp1.jsonvalue.s,std::make_shared<Value>(temp2.jsonvalue));
        jsonvalue.object.insert(a);
        parse_whitespace();
        if(*current == ',')
        {
            ++current;
            parse_whitespace();
        }
        else if(*current == '}')
        {
            ++current;
            jsonvalue.type=Value::OBJECT;
            return PARSE_OK;
        }
        else
        {
            ret = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    return ret;

}