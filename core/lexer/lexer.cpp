#include "lexer.h"
#include "utf8_utils.h"
#include <cctype>
#include <iostream>
#include <unordered_set>
#include <exception>


bool suppressNextTagEnd = false;


static constexpr bool ALPHA_TABLE[256] = {
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, true,
    false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
};

static constexpr bool DIGIT_TABLE[256] = {
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
};

static constexpr bool WHITESPACE_TABLE[256] = {
    false, false, false, false, false, false, false, false, false, true,  true,  true,  true,  true,  false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    true,  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
};


static const std::unordered_set<std::string> KEYWORDS = {
    
    "component", "import", "from", "export", "script",
    
    
    "if", "else", "elif", "while", "for", "in", "break", 
    "continue", "return", "match", "case", "default",
    
    
    "let", "const", "var", "func", "fn", "method",
    "class", "struct", "enum", "union", "trait",
    
    
    "new", "delete", "ref", "deref", "owned", "borrowed",
    "shared", "weak", "unique", "clone",
    
    
    "as", "is", "typeof", "sizeof", "alignof", "null",
    "undefined", "void", "any", "never", "unknown",
    
    
    "public", "private", "protected", "internal", "static",
    "final", "abstract", "virtual", "override",
    
    
    "try", "catch", "finally", "throw", "throws", "result",
    "error", "panic", "assert", "debug",
    
    
    "spawn", "join", "channel", "select", "send", "recv",
    "mutex", "atomic", "volatile", "sync",
    
    
    "true", "false", "this", "super", "self", "Self",
    
    
    "module", "namespace", "use", "pub", "crate", "mod",
    
    
    "meta", "attribute", "annotation", "deprecated",
    "inline", "noinline", "optimize", "unsafe", "safe",
    
    
    "print", "println", "log", "warn", "error"
};

static std::string toLower(const std::string &str)
{
    std::string lower;
    lower.reserve(str.size());
    for (char c : str)
        lower.push_back(std::tolower(static_cast<unsigned char>(c)));
    return lower;
}

void Lexer::enterState(LexerState newState)
{
    stateStack.push_back(state);
    state = newState;
}

void Lexer::exitState()
{
    if (!stateStack.empty())
    {
        state = stateStack.back();
        stateStack.pop_back();
    }
    else
    {
        state = LexerState::Normal;
    }
}

Lexer::Lexer(const std::string &source)
    : input(source), position(0), line(1), column(1), state(LexerState::Normal), isUTF8Error(false) 
{
    stateStack.reserve(8);
}

char Lexer::peek() const
{
    return eof() ? '\0' : input[position];
}

char Lexer::peekAdvance() const
{
    return (position + 1 < input.size()) ? input[position + 1] : '\0';
}

bool Lexer::eof() const
{
    return position >= input.size();
}

char Lexer::advance()
{
    if (eof())
        return '\0';
    char currentChar = input[position++];
    if (currentChar == '\n')
    {
        ++line;
        column = 1;
    }
    else
    {
        ++column;
    }
    return currentChar;
}

char Lexer::advanceRaw()
{
    if (eof())
        return '\0';
    char currentChar = input[position++];
    
    if (currentChar == '\n')
    {
        ++line;
        column = 1;
    }
    return currentChar;
}

void Lexer::advanceUTF8Character(int byteCount)
{
    for (int i = 0; i < byteCount && !eof(); ++i)
    {
        char c = input[position++];
        if (c == '\n')
        {
            ++line;
            column = 1;
            return; 
        }
    }
    ++column; 
}

bool Lexer::match(char expected)
{
    if (eof() || input[position] != expected)
        return false;
    ++position;
    ++column;
    return true;
}

Token Lexer::createErrorToken(const std::string &lexeme, const std::string &message)
{
    return Token(TokenType::Error, lexeme, line, column, message);
}

void Lexer::skipWhitespace()
{
    while (!eof())
    {
        char c = peek();

        if (WHITESPACE_TABLE[static_cast<unsigned char>(c)])
        {
            advance();
        }
        else if (c == '/' && peekAdvance() == '/')
        {
            
            advance(); 
            advance(); 
            while (!eof() && peek() != '\n')
                advance();
        }
        else if (c == '/' && peekAdvance() == '*')
        {
            
            advance(); 
            advance(); 
            bool terminated = false;
            while (!eof())
            {
                if (peek() == '*' && peekAdvance() == '/')
                {
                    advance(); 
                    advance(); 
                    terminated = true;
                    break;
                }
                advance();
            }
            if (!terminated)
            {
                break; 
            }
        }
        else
        {
            break;
        }
    }
}

std::optional<uint32_t> Lexer::decodeUTF8()
{
    if (position >= input.size())
        return std::nullopt;

    unsigned char lead = input[position];
    size_t remaining = input.size() - position;

    uint32_t codepoint = 0;
    size_t len = 0;

    if (lead <= 0x7F)
    {
        codepoint = lead;
        len = 1;
    }
    else if ((lead >> 5) == 0x6 && remaining >= 2)
    {
        codepoint = ((lead & 0x1F) << 6) |
                    (input[position + 1] & 0x3F);
        len = 2;
    }
    else if ((lead >> 4) == 0xE && remaining >= 3)
    {
        codepoint = ((lead & 0x0F) << 12) |
                    ((input[position + 1] & 0x3F) << 6) |
                    (input[position + 2] & 0x3F);
        len = 3;
    }
    else if ((lead >> 3) == 0x1E && remaining >= 4)
    {
        codepoint = ((lead & 0x07) << 18) |
                    ((input[position + 1] & 0x3F) << 12) |
                    ((input[position + 2] & 0x3F) << 6) |
                    (input[position + 3] & 0x3F);
        len = 4;
    }
    else
    {
        return std::nullopt;
    }

    position += len;
    column += 1;
    return codepoint;
}

bool Lexer::isDigit(char c) const
{
    return DIGIT_TABLE[static_cast<unsigned char>(c)];
}

bool Lexer::isAlpha(char c) const
{
    return ALPHA_TABLE[static_cast<unsigned char>(c)];
}

bool Lexer::isAlphaNumeric(char c) const
{
    unsigned char uc = static_cast<unsigned char>(c);
    return ALPHA_TABLE[uc] || DIGIT_TABLE[uc];
}

Token Lexer::processNumber()
{
    size_t startLine = line, startColumn = column;
    std::string number;
    number.reserve(16);
    bool hasDecimal = false;

    while (!eof() && (isDigit(peek()) || peek() == '.'))
    {
        char c = peek();
        if (c == '.')
        {
            if (hasDecimal)
            {
                number += advance();
                return createErrorToken(number, "Invalid number format: multiple decimal points");
            }
            hasDecimal = true;
        }
        number += advance();
    }
    
    return Token(TokenType::Number, std::move(number), startLine, startColumn);
}

Token Lexer::processIdentifierOrKeyword()
{
    size_t startLine = line, startColumn = column;
    std::string id;
    id.reserve(32);
    
    
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '-' || peek() == '_'))
        id += advance();

    
    bool followsNumber = false;
    if (startColumn > 1) {
        size_t checkPos = position - id.length() - 1;
        while (checkPos > 0 && WHITESPACE_TABLE[static_cast<unsigned char>(input[checkPos])]) {
            checkPos--;
        }
        if (checkPos < input.size() && isDigit(input[checkPos])) {
            followsNumber = true;
        }
    }
    
    
    if (!followsNumber && !id.empty() && !isDigit(id[0])) {
        std::string idLower = toLower(id);
        if (KEYWORDS.count(idLower)) {
            return Token(TokenType::Keyword, std::move(id), startLine, startColumn);
        }
    }
    
    return Token(TokenType::Identifier, std::move(id), startLine, startColumn);
}

Token Lexer::processString()
{
    size_t startLine = line, startColumn = column;
    std::string value;
    value.reserve(64);
    std::string rawLexeme = "\"";

    advance(); 

    while (!eof())
    {
        char c = peek();
        rawLexeme += c;

        if (c == '"')
        {
            advance();
            return Token(TokenType::String, std::move(value), startLine, startColumn);
        }

        if (c == '\n')
        {
            return createErrorToken(rawLexeme, "Unterminated string literal");
        }

        if (c == '\\')
        {
            advance();
            if (eof())
            {
                return createErrorToken(rawLexeme, "Unterminated string: escape at EOF");
            }
            char escaped = peek();
            rawLexeme += escaped;
            advance();

            switch (escaped)
            {
            case 'n': value += '\n'; break;
            case 't': value += '\t'; break;
            case 'r': value += '\r'; break;
            case '\\': value += '\\'; break;
            case '"': value += '"'; break;
            case '0': value += '\0'; break;
            default: value += escaped; break;
            }
        }
        else
        {
            value += advance();
        }

        if (value.length() > 100000)
        {
            return createErrorToken(rawLexeme.substr(0, 50) + "...", "String literal too long (>100k chars)");
        }
    }

    return createErrorToken(rawLexeme, "Unterminated string literal: reached EOF");
}

Token Lexer::processOperator()
{
    size_t startLine = line, startColumn = column;
    char first = peek();
    char second = peekAdvance();

    
    if ((first == '=' && second == '>') || (first == '-' && second == '>') ||
        (first == '=' && second == '=') || (first == '!' && second == '=') ||
        (first == '<' && second == '=') || (first == '>' && second == '=') ||
        (first == '&' && second == '&') || (first == '|' && second == '|'))
    {
        std::string op;
        op += advance();
        op += advance();

        if (op == "=>" || op == "->")
            return Token(TokenType::Arrow, std::move(op), startLine, startColumn);

        return Token(TokenType::Operator, std::move(op), startLine, startColumn);
    }

    
    char op = advance();
    switch (op)
    {
    case '=': return Token(TokenType::Equals, "=", startLine, startColumn);
    case '+': case '-': case '*': case '/': case '%':
    case '<': case '>': case '!':
        return Token(TokenType::Operator, std::string(1, op), startLine, startColumn);
    case '(': return Token(TokenType::ParenOpen, "(", startLine, startColumn);
    case ')': return Token(TokenType::ParenClose, ")", startLine, startColumn);
    case '[': return Token(TokenType::BracketOpen, "[", startLine, startColumn);
    case ']': return Token(TokenType::BracketClose, "]", startLine, startColumn);
    case ':': return Token(TokenType::Colon, ":", startLine, startColumn);
    case ';': return Token(TokenType::SemiColon, ";", startLine, startColumn);
    case ',': return Token(TokenType::Comma, ",", startLine, startColumn);
    case '.': return Token(TokenType::Dot, ".", startLine, startColumn);
    default: return Token(TokenType::Unknown, std::string(1, op), startLine, startColumn);
    }
}

bool Lexer::isOperatorStartChar(char c) const
{
    static const std::unordered_set<char> validOps = {
        '=', '!', '<', '>', '+', '-', '*', '/', '%', '&', '|', ':',
        '(', ')', '{', '}', '[', ']', ',', '.', ';'};
    return validOps.count(c) > 0;
}

Token Lexer::processTag()
{
    size_t startLine = line, startColumn = column;
    std::string tagContent = "<";
    advance(); 

    std::string tag;
    tag.reserve(32);
    
    
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '-'))
    {
        char c = advance();
        tag += c;
        tagContent += c;

        if (tag.length() > 200)
        {
            return createErrorToken(tagContent, "Tag name too long");
        }
    }

    if (tag.empty())
    {
        return createErrorToken("<", "Invalid tag: expected tag name after '<'");
    }

    enterState(LexerState::ALTXAttribute);
    return Token(TokenType::TagOpen, std::move(tag), startLine, startColumn);
}

Token Lexer::processTagEnd()
{
    size_t startLine = line, startColumn = column;
    advance(); 
    advance(); 

    std::string tag;
    tag.reserve(32);
    
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '-'))
        tag += advance();

    suppressNextTagEnd = true;
    if (state == LexerState::ALTXContent)
    {
        exitState();
    }

    return Token(TokenType::TagClose, std::move(tag), startLine, startColumn);
}

Token Lexer::processTextContent()
{
    size_t startLine = line, startColumn = column;
    std::string text;
    text.reserve(64);

    while (!eof())
    {
        char c = peek();
        if (c == '<' || c == '{' || c == ':' || WHITESPACE_TABLE[static_cast<unsigned char>(c)])
        {
            break;
        }
        text += advance();
    }

    return Token(TokenType::Text, std::move(text), startLine, startColumn);
}


Token Lexer::processValueBinding()
{
    size_t startLine = line, startColumn = column;
    advance(); 
    
    std::string identifier;
    identifier.reserve(32);
    
    
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '_'))
    {
        identifier += advance();
    }
    
    if (identifier.empty())
    {
        return createErrorToken("!", "Invalid value binding: expected identifier after '!'");
    }
    
    return Token(TokenType::ValueBinding, "!" + identifier, startLine, startColumn);
}


Token Lexer::processStyleProperty()
{
    size_t startLine = line, startColumn = column;
    std::string styleContent;
    styleContent.reserve(128);
    
    
    skipWhitespace();
    
    while (!eof())
    {
        char c = peek();
        
        
        if (c == '>' || c == '/' || WHITESPACE_TABLE[static_cast<unsigned char>(c)])
        {
            
            size_t tempPos = position;
            while (tempPos < input.size() && WHITESPACE_TABLE[static_cast<unsigned char>(input[tempPos])])
                tempPos++;
            
            if (tempPos < input.size() && isAlpha(input[tempPos]))
                break; 
        }
        
        if (c == '>' || c == '/')
            break;
            
        styleContent += advance();
    }
    
    
    while (!styleContent.empty() && WHITESPACE_TABLE[static_cast<unsigned char>(styleContent.back())])
        styleContent.pop_back();
    
    return Token(TokenType::StyleProperty, std::move(styleContent), startLine, startColumn);
}

void Lexer::recoverFromError()
{
    while (!eof())
    {
        size_t start = position;
        unsigned char first = static_cast<unsigned char>(peek());

        
        if (WHITESPACE_TABLE[first] || first == ';' || first == '{' || first == '}' || first == '<' || first == '>')
            break;

        if (ALPHA_TABLE[first] || DIGIT_TABLE[first])
            break;

        int expectedBytes = 1;
        if ((first & 0xE0) == 0xC0) expectedBytes = 2;
        else if ((first & 0xF0) == 0xE0) expectedBytes = 3;
        else if ((first & 0xF8) == 0xF0) expectedBytes = 4;

        advanceUTF8Character(expectedBytes);

        
        if (position == start)
        {
            ++position;
            ++column;
        }

        break; 
    }
}

Token Lexer::nextToken()
{
    skipWhitespace();
    isUTF8Error = false;

    if (eof())
        return Token(TokenType::EOFToken, "", line, column);

    size_t startLine = line, startColumn = column;
    char c = peek();

    
    if (c == '!' && isAlpha(peekAdvance()))
    {
        return processValueBinding();
    }

    
    if (c == '<' && peekAdvance() == '/')
        return processTagEnd();

    if (c == '<' && isAlpha(peekAdvance()))
        return processTag();

    if (c == '>')
    {
        if (suppressNextTagEnd)
        {
            suppressNextTagEnd = false;
            advance();
            return nextToken();
        }

        advance();
        if (state == LexerState::ALTXAttribute)
        {
            exitState();
            enterState(LexerState::ALTXContent);
        }
        return Token(TokenType::TagEnd, ">", startLine, startColumn);
    }

    
    if (c == '/' && peekAdvance() == '>')
    {
        advance(); 
        advance(); 
        if (state == LexerState::ALTXAttribute)
        {
            exitState();
        }
        return Token(TokenType::TagSelfClose, "/>", startLine, startColumn);
    }

    
    if (state == LexerState::ALTXAttribute)
    {
        if (c == '=')
        {
            advance();
            return Token(TokenType::Equals, "=", startLine, startColumn);
        }

        if (isAlpha(c))
        {
            
            if (position + 5 < input.size() && 
                input.substr(position, 5) == "style")
            {
                size_t nextPos = position + 5;
                if (nextPos < input.size() && input[nextPos] == ':')
                {
                    
                    position += 6;
                    column += 6;
                    return processStyleProperty();
                }
            }
            
            
            std::string attrName;
            attrName.reserve(32);
            while (!eof() && (isAlphaNumeric(peek()) || peek() == '-'))
                attrName += advance();
            return Token(TokenType::AttributeName, std::move(attrName), startLine, startColumn);
        }

        if (c == '"')
            return processString();

        if (c == '{')
        {
            advance();
            enterState(LexerState::Expression);
            return Token(TokenType::ExpressionStart, "{", startLine, startColumn);
        }
    }

    
    if (state == LexerState::ALTXContent)
    {
        if (c == '<')
        {
            if (peekAdvance() == '/')
                return processTagEnd();
            else if (isAlpha(peekAdvance()))
                return processTag();
        }

        if (c == '{')
        {
            advance();
            enterState(LexerState::Expression);
            return Token(TokenType::ExpressionStart, "{", startLine, startColumn);
        }

        if (isAlpha(c) || isDigit(c))
            return processTextContent();
    }

    
    if (c == '{')
    {
        advance();
        if (state == LexerState::Expression || state == LexerState::Normal)
            return Token(TokenType::BraceOpen, "{", startLine, startColumn);
        else
        {
            enterState(LexerState::Expression);
            return Token(TokenType::ExpressionStart, "{", startLine, startColumn);
        }
    }

    if (c == '}')
    {
        advance();
        if (state == LexerState::Expression)
        {
            exitState();
            return Token(TokenType::ExpressionEnd, "}", startLine, startColumn);
        }
        else
            return Token(TokenType::BraceClose, "}", startLine, startColumn);
    }

    
    if (c == '@')
    {
        advance();
        std::string modifierName = "@";
        modifierName.reserve(32);
        while (!eof() && (isAlphaNumeric(peek()) || peek() == '_'))
            modifierName += advance();
        return Token(TokenType::AtModifier, std::move(modifierName), startLine, startColumn);
    }

    
    if (isDigit(c))
        return processNumber();

    
    if (isAlpha(c))
        return processIdentifierOrKeyword();

    
    if (c == '"')
        return processString();

    
    if (isOperatorStartChar(c))
        return processOperator();

    
    if (static_cast<unsigned char>(c) > 127)
    {
        isUTF8Error = true;
        size_t startCol = column;
        unsigned char first = static_cast<unsigned char>(c);
        int expectedBytes = 1;

        if ((first & 0x80) == 0x00) expectedBytes = 1;
        else if ((first & 0xE0) == 0xC0) expectedBytes = 2;
        else if ((first & 0xF0) == 0xE0) expectedBytes = 3;
        else if ((first & 0xF8) == 0xF0) expectedBytes = 4;
        else
        {
            advance();
            return createErrorToken(std::string(1, c), "Invalid UTF-8 start byte");
        }

        if (position + expectedBytes > input.size())
        {
            std::string partial;
            while (!eof() && static_cast<unsigned char>(peek()) > 127)
            {
                partial += advance();
            }
            return createErrorToken(partial, "Incomplete UTF-8 sequence");
        }

        std::string utf8Bytes;
        utf8Bytes.reserve(expectedBytes);
        bool valid = true;

        for (int i = 0; i < expectedBytes && position + i < input.size(); ++i)
        {
            unsigned char byte = static_cast<unsigned char>(input[position + i]);
            if (i == 0)
            {
                utf8Bytes += static_cast<char>(byte);
            }
            else
            {
                if ((byte & 0xC0) != 0x80)
                {
                    valid = false;
                    break;
                }
                utf8Bytes += static_cast<char>(byte);
            }
        }

        if (!valid)
        {
            advance();
            return createErrorToken(std::string(1, c), "Invalid UTF-8 continuation byte");
        }

        advanceUTF8Character(expectedBytes);
        return Token(TokenType::Error, std::move(utf8Bytes), startLine, startCol,
                     "UTF-8 character encountered (ALTERION uses ASCII): " + utf8Bytes);
    }
    else if (c < 32 && c != '\t' && c != '\n' && c != '\r')
    {
        advance();
        return createErrorToken(std::string(1, c), "Invalid control character (code " + std::to_string((int)c) + ")");
    }
    else
    {
        advance();
        return createErrorToken(std::string(1, c), "Unexpected character");
    }
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    tokens.reserve(256);
    int errorCount = 0;
    const int MAX_ERRORS = 1000;

    while (!eof() && errorCount < MAX_ERRORS)
    {
        Token token = safeNextToken();
        tokens.push_back(std::move(token));

        if (tokens.back().type == TokenType::Error)
        {
            errorCount++;
            
            
            if (!isUTF8Error)
            {
                recoverFromError();
            }
        }

        if (tokens.back().type == TokenType::EOFToken)
            break;
    }

    if (errorCount >= MAX_ERRORS)
    {
        tokens.push_back(createErrorToken("", "Too many lexer errors, stopping"));
    }

    if (tokens.empty() || tokens.back().type != TokenType::EOFToken)
    {
        tokens.push_back(Token(TokenType::EOFToken, "", line, column));
    }

    return tokens;
}

std::string Lexer::getTokenTypeName(TokenType type) const
{
    switch (type)
    {
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
    case TokenType::Text: return "Text";
    case TokenType::AttributeName: return "AttributeName";
    case TokenType::AttributeValue: return "AttributeValue";
    case TokenType::Equals: return "Equals";
    case TokenType::ExpressionStart: return "ExpressionStart";
    case TokenType::ExpressionEnd: return "ExpressionEnd";
    case TokenType::BraceOpen: return "BraceOpen";
    case TokenType::BraceClose: return "BraceClose";
    case TokenType::Colon: return "Colon";
    case TokenType::SemiColon: return "SemiColon";
    case TokenType::ParenOpen: return "ParenOpen";
    case TokenType::ParenClose: return "ParenClose";
    case TokenType::BracketOpen: return "BracketOpen";
    case TokenType::BracketClose: return "BracketClose";
    case TokenType::Comma: return "Comma";
    case TokenType::Dot: return "Dot";
    case TokenType::EOFToken: return "EOF";
    case TokenType::AtModifier: return "AtModifier";
    case TokenType::ValueBinding: return "ValueBinding";
    case TokenType::StyleProperty: return "StyleProperty";
    case TokenType::Error: return "Error";
    case TokenType::ErrorRecovery: return "ErrorRecovery";
    case TokenType::Unknown: return "Unknown";
    default: return "Unknown";
    }
}

void Lexer::debugPrintTokens(const std::vector<Token> &tokens) const
{
    int errorCount = 0;
    for (const auto &token : tokens)
    {
        if (token.type == TokenType::Error)
        {
            std::cout << "ERROR: " << token.value;
            if (!token.errorMessage.empty())
                std::cout << " (" << token.errorMessage << ")";
            std::cout << " at line " << token.line << ", col " << token.column << std::endl;
            errorCount++;
        }
        else
        {
            std::cout << getTokenTypeName(token.type) << ": '" << token.value
                      << "' (L" << token.line << ":C" << token.column << ")" << std::endl;
        }
    }

    if (errorCount > 0)
    {
        std::cout << "\nTotal errors: " << errorCount << std::endl;
    }
}

Token Lexer::safeNextToken() {
    try {
        return nextToken();
    } catch (const std::exception& e) {
        return createErrorToken("", std::string("Lexer exception: ") + e.what());
    } catch (...) {
        return createErrorToken("", "Unknown lexer error");
    }
}