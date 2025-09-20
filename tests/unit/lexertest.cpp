#include "../../core/include/lexer.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <unordered_set>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <sstream>

// Structure to hold expected token info
struct ExpectedToken {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
};

// Converts TokenType to string for output
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::Keyword: return "Keyword";
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        case TokenType::Operator: return "Operator";
        case TokenType::Arrow: return "Arrow";
        case TokenType::TagOpen: return "TagOpen";
        case TokenType::TagClose: return "TagClose";
        case TokenType::TagSelfClose: return "TagSelfClose";
        case TokenType::TagEnd: return "TagEnd";
        case TokenType::AttributeName: return "AttributeName";
        case TokenType::AttributeValue: return "AttributeValue";
        case TokenType::Text: return "Text";
        case TokenType::Comment: return "Comment";
        case TokenType::ExpressionStart: return "ExpressionStart";
        case TokenType::ExpressionEnd: return "ExpressionEnd";
        case TokenType::Equals: return "Equals";
        case TokenType::BraceOpen: return "BraceOpen";
        case TokenType::BraceClose: return "BraceClose";
        case TokenType::Colon: return "Colon";
        case TokenType::SemiColon: return "SemiColon";
        case TokenType::ParenOpen: return "ParenOpen";
        case TokenType::ParenClose: return "ParenClose";
        case TokenType::SquareBracketOpen: return "SquareBracketOpen";
        case TokenType::SquareBracketClose: return "SquareBracketClose";
        case TokenType::Comma: return "Comma";
        case TokenType::Dot: return "Dot";
    case TokenType::AtModifier: return "AtModifier";
    case TokenType::AtAsyncModifier: return "AtAsyncModifier";
        case TokenType::ValueBinding: return "ValueBinding";
        case TokenType::StyleProperty: return "StyleProperty";
        case TokenType::EOFToken: return "EOFToken";
        case TokenType::Unknown: return "Unknown";
        case TokenType::Error: return "Error";
        case TokenType::ErrorRecovery: return "ErrorRecovery";
        default: return "Unknown";
    }
}

// Compute expected type/value logically for each token
void computeExpected(const Token& token, std::string& expectedType, std::string& expectedValue) {
    static const std::unordered_set<std::string> keywords = {
        "async", "component", "import", "extern", "for", "if", "else", "while", "return", "break", "continue", "yield", "await", "throw", "try", "catch", "finally",
        "render", "script", "function", "fn", "method", "type", "interface", "struct", "enum", "union", "from", "export", "use", "pub", "mod", "namespace",
        "move", "copy", "ref", "deref", "owned", "borrowed", "shared", "weak", "true", "false", "null", "none", "undefined", "this", "super", "self", "Self", "match", "case", "default",
        "let", "const", "var", "new", "delete", "as", "is", "typeof", "print", "println"
    };

    // Predict expected type for comments
    if (token.type == TokenType::Comment) {
        expectedType = "Comment";
        expectedValue = token.value;
        return;
    }

    // Predict expected type for keywords
    if (keywords.count(token.value)) {
        expectedType = "Keyword";
        expectedValue = token.value;
        return;
    }

    // Predict expected type for numbers
    if (token.type == TokenType::Number) {
        expectedType = "Number";
        expectedValue = token.value;
        return;
    }

    // Predict expected type for strings
    if (token.type == TokenType::String) {
        expectedType = "String";
        expectedValue = token.value;
        return;
    }

    // Predict expected type for identifiers
    if (token.type == TokenType::Identifier) {
        expectedType = "Identifier";
        expectedValue = token.value;
        return;
    }

    // Predict expected type for operators and arrows
    if (token.type == TokenType::Operator || token.type == TokenType::Arrow) {
        expectedType = tokenTypeToString(token.type);
        expectedValue = token.value;
        return;
    }

    // Predict expected type for tags
    if (token.type == TokenType::TagOpen || token.type == TokenType::TagClose || token.type == TokenType::TagSelfClose || token.type == TokenType::TagEnd) {
        expectedType = tokenTypeToString(token.type);
        expectedValue = token.value;
        return;
    }

    // Predict expected type for attributes
    if (token.type == TokenType::AttributeName || token.type == TokenType::AttributeValue) {
        expectedType = tokenTypeToString(token.type);
        expectedValue = token.value;
        return;
    }

    // Predict expected type for text
    if (token.type == TokenType::Text) {
        expectedType = "Text";
        expectedValue = token.value;
        return;
    }

    // Predict expected type for expressions
    if (token.type == TokenType::ExpressionStart || token.type == TokenType::ExpressionEnd) {
        expectedType = tokenTypeToString(token.type);
        expectedValue = token.value;
        return;
    }

    // Predict expected type for punctuation
    if (token.type == TokenType::Equals || token.type == TokenType::BraceOpen || token.type == TokenType::BraceClose || token.type == TokenType::Colon || token.type == TokenType::SemiColon || token.type == TokenType::ParenOpen || token.type == TokenType::ParenClose || token.type == TokenType::SquareBracketOpen || token.type == TokenType::SquareBracketClose || token.type == TokenType::Comma || token.type == TokenType::Dot) {
        expectedType = tokenTypeToString(token.type);
        expectedValue = token.value;
        return;
    }

    // Predict expected type for at-modifier
    if (token.type == TokenType::AtModifier) {
        expectedType = "AtModifier";
        expectedValue = token.value;
        return;
    }
    // Predict expected type for at-async-modifier
    if (token.type == TokenType::AtAsyncModifier) {
        expectedType = "AtAsyncModifier";
        expectedValue = token.value;
        return;
    }

    // Predict expected type for value binding
    if (token.type == TokenType::ValueBinding) {
        expectedType = "ValueBinding";
        expectedValue = token.value;
        return;
    }

    // Predict expected type for style property
    if (token.type == TokenType::StyleProperty) {
        expectedType = "StyleProperty";
        expectedValue = token.value;
        return;
    }

    // Predict expected type for EOF
    if (token.type == TokenType::EOFToken) {
        expectedType = "EOFToken";
        expectedValue = "";
        return;
    }

    // Predict expected type for errors
    if (token.type == TokenType::Error || token.type == TokenType::ErrorRecovery) {
        expectedType = tokenTypeToString(token.type);
        expectedValue = token.value;
        return;
    }

    // Fallback for unknowns
    expectedType = "Unknown";
    expectedValue = token.value;
}

// Helper to escape JSON string values
std::string jsonEscape(const std::string& s) {
    std::string result;
    for (unsigned char c : s) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    result += buf;
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

int main() {
    
    std::ifstream file("examples/lexer-app-test.alt");
    if (!file) {
        std::cerr << "Failed to open examples/lexer-app-test.alt" << std::endl;
        return 1;
    }
    std::string input((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    std::cout << "[DEBUG] Input size: " << input.size() << " bytes\n";

    Lexer lexer(input);
    std::vector<Token> actualTokens = lexer.tokenize();
    std::cout << "[DEBUG] Token count: " << actualTokens.size() << "\n";


    std::filesystem::create_directories("results");

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char filename[128];
    std::strftime(filename, sizeof(filename), "results-dashboard/public/results/lexer-results.json", std::localtime(&now_time));

    // Output HTML
    std::ofstream json(filename);
    if (!json) {
        std::cerr << "[ERROR] Could not open " << filename << " for writing!\n";
        return 2;
    }
    std::cout << "[DEBUG] Writing JSON to " << filename << "...\n";
    json << "[\n"; 
    for (size_t i = 0; i < actualTokens.size(); ++i) {
        const Token& token = actualTokens[i];
        std::string expectedType, expectedValue;
        computeExpected(token, expectedType, expectedValue);
        std::string returnedType;
        if (token.type == TokenType::Text) {
            returnedType = "Text";
        } else {
            returnedType = tokenTypeToString(token.type);
        }
        std::string returnedValue = token.value;
        std::string status = (expectedType == returnedType && expectedValue == returnedValue) ? "OK" : "DIFF";
        json << "  {\n"
             << "    \"index\": " << i << ",\n"
             << "    \"expectedType\": \"" << jsonEscape(expectedType) << "\",\n"
             << "    \"expectedValue\": \"" << jsonEscape(expectedValue) << "\",\n"
             << "    \"returnedType\": \"" << jsonEscape(returnedType) << "\",\n"
             << "    \"returnedValue\": \"" << jsonEscape(returnedValue) << "\",\n"
             << "    \"line\": " << token.line << ",\n"
             << "    \"column\": " << token.column << ",\n"
             << "    \"status\": \"" << jsonEscape(status) << "\"\n"
             << "  }" << (i < actualTokens.size() - 1 ? "," : "") << "\n"; 
    }
    json << "]\n";
    json.close();
    std::cout << "Lexer test complete. Output written to " << filename << "\n";
    return 0;
}