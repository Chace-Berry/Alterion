#include "token.h"
#include <iostream>  

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << "[";
    switch (token.type) {
        case TokenType::Identifier:       os << "Identifier"; break;
        case TokenType::Keyword:          os << "Keyword"; break;
        case TokenType::Number:           os << "Number"; break;
        case TokenType::String:           os << "String"; break;
        case TokenType::Operator:         os << "Operator"; break;
        case TokenType::Arrow:            os << "Arrow"; break;

        case TokenType::BraceOpen:        os << "BraceOpen"; break;
        case TokenType::BraceClose:       os << "BraceClose"; break;
        case TokenType::ParenOpen:        os << "ParenOpen"; break;
        case TokenType::ParenClose:       os << "ParenClose"; break;
        case TokenType::SquareBracketOpen:      os << "SquareBracketOpen"; break;
        case TokenType::SquareBracketClose:     os << "SquareBracketClose"; break;

        case TokenType::Comma:            os << "Comma"; break;
        case TokenType::Dot:              os << "Dot"; break;
        case TokenType::Colon:            os << "Colon"; break;
        case TokenType::SemiColon:        os << "SemiColon"; break;
        case TokenType::Equals:           os << "Equals"; break;

        case TokenType::TagOpen:          os << "TagOpen"; break;
        case TokenType::TagEnd:           os << "TagEnd"; break;
        case TokenType::TagClose:         os << "TagClose"; break;
        case TokenType::TagSelfClose:     os << "TagSelfClose"; break;

        case TokenType::AttributeName:    os << "AttributeName"; break;
        case TokenType::AttributeValue:   os << "AttributeValue"; break;

        case TokenType::ExpressionStart:  os << "ExpressionStart"; break;
        case TokenType::ExpressionEnd:    os << "ExpressionEnd"; break;

    case TokenType::AtModifier:       os << "AtModifier"; break;
    case TokenType::AtAsyncModifier:  os << "AtAsyncModifier"; break;
        case TokenType::ValueBinding:     os << "ValueBinding"; break;
        case TokenType::StyleProperty:    os << "StyleProperty"; break;

        case TokenType::Text:             os << "Text"; break;
        case TokenType::EOFToken:         os << "EOFToken"; break;
        case TokenType::Error:            os << "Error"; break;
        case TokenType::ErrorRecovery:    os << "ErrorRecovery"; break;
        case TokenType::Unknown:          os << "Unknown"; break;

        default:                         os << "Other"; break;
    }
    os << ": '" << token.value << "' (Line " << token.line << ", Col " << token.column << ")]";
    return os;
}