#include <stdio.h>
#include <stdlib.h>

typedef enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_PRIMARY
} Precedence;

typedef enum TokenKind {
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
} TokenKind;

typedef struct Token {
    TokenKind kind;
    char* start;
    int length;
} Token;

typedef enum ASTNodeType {
    AST_BINARY,
    AST_UNARY,
    AST_NUMBER,
} ASTNodeType;

typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    ASTNodeType type;
    Token token;
} ASTNode;

typedef struct Lexer {
    char* buffer;
    char* current;
} Lexer;

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

void lexer_skip_whitespace(Lexer* lexer) {
    while (*lexer->current == ' ' || *lexer->current == '\n' ||
           *lexer->current == '\r' || *lexer->current == '\t') {
        lexer->current++;
    }
}

Token next_token(Lexer* lexer) {
    lexer_skip_whitespace(lexer);

    Token token;
    token.start = lexer->current;
    token.length = 1;

    switch (*lexer->current) {
        case '\0':
            token.kind = TOKEN_EOF;
            break;
        case '+':
            token.kind = TOKEN_PLUS;
            break;
        case '-':
            token.kind = TOKEN_MINUS;
            break;
        case '*':
            token.kind = TOKEN_STAR;
            break;
        case '/':
            token.kind = TOKEN_SLASH;
            break;
        case '(':
            token.kind = TOKEN_LPAREN;
            break;
        case ')':
            token.kind = TOKEN_RPAREN;
            break;
        default:
            if (*lexer->current >= '0' && *lexer->current <= '9') {
                token.kind = TOKEN_NUMBER;
                while (*(lexer->current + token.length) >= '0' &&
                       *(lexer->current + token.length) <= '9') {
                    token.length++;
                }
            } else {
                printf("Unexpected character: %c\n", *lexer->current);
                exit(1);
            }
            break;
    }

    lexer->current += token.length;
    return token;
}

ASTNode* create_ast_node(ASTNodeType type, Token token) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->left = NULL;
    node->right = NULL;
    node->type = type;
    node->token = token;
    return node;
}

ASTNode* parse_expression(Lexer* lexer, Precedence precedence) {
    Token token = next_token(lexer);
    ASTNode* left = NULL;

    switch (token.kind) {
        case TOKEN_NUMBER:
            left = create_ast_node(AST_NUMBER, token);
            break;
        case TOKEN_LPAREN:
            left = parse_expression(lexer, PREC_ASSIGNMENT);
            if (next_token(lexer).kind != TOKEN_RPAREN) {
                printf("Expected ')'\n");
                exit(1);
            }
            break;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            left = create_ast_node(AST_UNARY, token);
            left->left = parse_expression(lexer, PREC_UNARY);
            break;
        default:
            printf("Unexpected token: %.*s\n", token.length, token.start);
            exit(1);
    }

    while (1) {
        Token next = next_token(lexer);
        Precedence next_precedence = PREC_NONE;

        switch (next.kind) {
            case TOKEN_PLUS:
            case TOKEN_MINUS:
                next_precedence = PREC_TERM;
                break;
            case TOKEN_STAR:
            case TOKEN_SLASH:
                next_precedence = PREC_FACTOR;
                break;
            default:
                lexer->current -= next.length;
                return left;
        }

        if (next_precedence <= precedence) {
            lexer->current -= next.length;
            return left;
        }

        ASTNode* binary = create_ast_node(AST_BINARY, next);
        binary->left = left;
        binary->right = parse_expression(lexer, next_precedence);
        left = binary;
    }
}

void init_lexer(Lexer* lexer, char* buffer) {
    lexer->buffer = buffer;
    lexer->current = buffer;
}

void print_ast(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_BINARY:
            printf("(%.*s ", node->token.length, node->token.start);
            print_ast(node->left);
            printf(" ");
            print_ast(node->right);
            printf(")");
            break;
        case AST_UNARY:
            printf("(%.*s ", node->token.length, node->token.start);
            print_ast(node->left);
            printf(")");
            break;
        case AST_NUMBER:
            printf("%.*s", node->token.length, node->token.start);
            break;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(file_size + 1);  // +1 for null terminator
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';  // Null-terminate the buffer

    fclose(file);

    printf("Input: \n");
    printf("%s\n", buffer);

    Lexer lexer;
    init_lexer(&lexer, buffer);

    ASTNode* root = parse_expression(&lexer, PREC_ASSIGNMENT);
    printf("Parsed expression:\n");
    print_ast(root);
    printf("\n");

    free_ast(root);
    free(buffer);

    return 0;
}
