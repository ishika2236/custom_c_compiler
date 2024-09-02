#include "compiler.h"


bool is_token_keyword(struct token* token, char* keyword)
{
    return token->type == TOKEN_TYPE_OPERATOR && token->sval == keyword;
}