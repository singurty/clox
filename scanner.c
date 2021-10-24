#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

Scanner scanner;

void initScanner(const char* source) {
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
	scanner.insideInterpolation = false;
}

static bool isAtEnd() {
	return *scanner.current == '\0';
}

static Token makeToken(TokenType type) {
	Token token;
	token.type = type;
	token.start = scanner.start;
	token.length = scanner.current - scanner.start;
	token.line = scanner.line;
	return token;
}

static Token errorToken(const char* message) {
	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = strlen(message);
	token.line = scanner.line;
	return token;
}

// consumes the next character and spits it out
// to consume means to increase the next character pointer
static char advance() {
	scanner.current++;
	return scanner.current[-1];
}

// get the next char without consuming it
static char peek() {
	return *scanner.current;
}

static char peekNext() {
	// check if the previous char was EOF before
	// returning the char after it
	if (isAtEnd()) return '\0';
	return scanner.current[1];
}

static bool match(char expected) {
	if (isAtEnd()) return false;
	if (*scanner.current != expected) return false;
	scanner.current++;
	return true;
}

static void skipWhitespace() {
	for (;;) {
		char c = peek();
		switch (c) {
		case ' ':
		case '\r':
		case '\t':
			advance();
			break;
		case '\n':
			scanner.line++;
			advance();
			break;
		case '/':
			if (peekNext() == '/') {
				// donot consume \n so line count will be incremented
				// in next loop
				while (peek() != '\n' && !isAtEnd()) advance();
			}
		default:
			return;
		}
	}
}

static Token string() {
	while (peek() != '"' && !isAtEnd()) {
		if (peek() == '\n') {
			scanner.line++;
		}
		// check for interpolation
		if (peek() == '$'&& peekNext() == '{') {
			scanner.insideInterpolation = true;
			return makeToken(TOKEN_INTERPOLATION);
		}
		advance();
	}
	if (isAtEnd()) return makeToken(TOKEN_ERROR);
	// consume the terminating quote
	advance();
	// the string literal can later be derived
	// from token.start and token.length
	return makeToken(TOKEN_STRING);
}

static bool isDigit(char c) {
	return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
	return (c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c == '_');
}

static Token number() {
	// consume all following digits
	while (isDigit(peek())) advance();
	// handle fractions
	// consume the . and the numbers after it
	if (peek() == '.' && isDigit(peekNext())) {
		advance();
		while (isDigit(peek())) advance();
	}
	return makeToken(TOKEN_NUMBER);
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
	// the full identifier has already been consumed at this point
	if (scanner.current - scanner.start == start + length &&
			memcmp(scanner.start + start, rest, length) == 0) {
		return type;
	}
	return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
	// trie for checking reserved words
	//
	// the identifier starts at scanner.start and ends just before
	// scanner.current
	switch (scanner.start[0]) {
		case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
		case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
		case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
		case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
		case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
		case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
		case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
		case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
		case 's': return checkKeyword(1, 3, "uper", TOKEN_SUPER);
		case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
		case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
		case 'f':
			if (scanner.current - scanner.start > 1) {
				switch (scanner.start[1]) {
					case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
					case 'o': return checkKeyword(2, 1, "r", TOKEN_OR);
					case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
				}
			}
		case 't':
			if (scanner.current - scanner.start > 1) {
				switch (scanner.start[1]) {
					case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
					case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
				}
			}
	}
	return TOKEN_IDENTIFIER;
}

static Token identifier() {
	// an identifier may have digits in it's name even though
	// should not start with one
	while (isAlpha(peek()) || isDigit(peek())) advance();
	return makeToken(identifierType());
}

Token scanToken() {
	skipWhitespace();
	scanner.start = scanner.current;
	if (isAtEnd()) return makeToken(TOKEN_EOF);
	char c = advance();
	// ignore "${" which was used to start interpolation
	if (peek() == '$' && peekNext() == '{') {
		advance();
		advance();
	}
	if (isDigit(c)) return number();
	if (isAlpha(c)) return identifier();

	switch (c) {
	case '(': return makeToken(TOKEN_LEFT_PAREN);
	case ')': return makeToken(TOKEN_RIGHT_PAREN);
	case '{': return makeToken(TOKEN_LEFT_BRACE);
	case '}':
		// used to end interpolation
		if (scanner.insideInterpolation) {
			scanner.insideInterpolation = false;
		    return string();
		}
		return makeToken(TOKEN_RIGHT_BRACE);
	case ';': return makeToken(TOKEN_SEMICOLON);
	case ',': return makeToken(TOKEN_COMMA);
	case '.': return makeToken(TOKEN_DOT);
	case '-': return makeToken(TOKEN_MINUS);
	case '+': return makeToken(TOKEN_PLUS);
	case '/': return makeToken(TOKEN_SLASH);
	case '*': return makeToken(TOKEN_STAR);
	case '!':
		return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
	case '=':
		return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
	case '<':
		return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
	case '>':
		return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
	case '"': return string();
	}
	return errorToken("Unexpected character.");
}
