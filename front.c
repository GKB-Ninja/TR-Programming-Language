/* front.c - a lexical analyzer system for simple arithmetic expressions */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

/***  Global Declarations  ***/

/* Variables */
int charClass;
wchar_t lexeme[100];
wchar_t nextChar;
int lexLen;
int token;
int nextToken;
FILE *in_fp;
wchar_t errMsg[256] = L"No errors found. This source code belongs to TR-701";

/* Function declarations */
void addChar();
void getChar();
void getNonBlank();
int lex();
int lookup(int compareMode);
void error(const wchar_t *message); // Universal error function

void program();
void statementList();
void statement();
void controlStatement();

void expr();
void factor();
void term();
void charLit();

void boolExpr();
void boolTerm();
void boolEquality();
void boolRelational();
void boolFactor();
void boolNot();

void ifStmt();
void whileStmt();
void forStmt();
void returnStmt();
void declStmt();
void assignStmt();

/* Character classes */
#define DIGIT 0
#define LETTER 1
#define UNKNOWN 2

/* Token codes */
#define INT_LIT 10
#define FP_LIT 11
#define IDENT 12
#define TYPE_INT 13
#define TYPE_FLOAT 14
#define TYPE_DOUBLE 15
#define TYPE_CHAR 16
#define TYPE_BOOL 17
#define TRUE_VAL 18
#define FALSE_VAL 19
#define ASSIGN_OP 20
#define EQUALITY_OP 21
#define NOT_EQUALITY_OP 22
#define LE_OP 23
#define GE_OP 24
#define LT_OP 25
#define GT_OP 26
#define NOT_OP 27
#define AND_OP 28
#define OR_OP 29
#define ADD_OP 30
#define SUB_OP 31
#define MULT_OP 32
#define DIV_OP 33
#define POWER_OP 34
#define MOD_OP 35
#define IF_CODE 40
#define ELSE_CODE 41
#define WHILE_CODE 42
#define FOR_CODE 43
#define CONTINUE_CODE 44
#define BREAK_CODE 45
#define LEFT_PAREN 60
#define RIGHT_PAREN 61
#define LEFT_CURLY 62
#define RIGHT_CURLY 63
#define LEFT_SQUARE 64
#define RIGHT_SQUARE 65
#define RETURN_CODE 70
#define EOL 90
#define COMMA 91
#define APOSTROPHE 92
#define COMMENT 93
#define UNREGISTERED_SYMBOL 99

/* Extras */
#define TURKISH_LETTER_MODE 4
#define OPERATOR_MODE 5
#define KEYWORD_MODE 6

/************************************************************************************/

/* main driver */
int main() {
    //setlocale(LC_CTYPE, "");
    if ((in_fp = _wfopen(L"front.in", L"rb")) == NULL)
        perror("front.in is not in the executable's directory or cannot be opened ");
    else {
        wchar_t BOM = fgetwc(in_fp); // Assume the first character is a BOM (and skip it)
        if (BOM != 0xFEFF) { // Check if the BOM is UTF-16LE and quit if not
            printf("front.in is not in UTF-16LE format.\n");
            fclose(in_fp);
            return 1;
        }

        getChar();
        lex();
        program();
        fclose(in_fp);
    }
}

/************************************************************************************/

/* error - a universal error handling function */
void error(const wchar_t *message) {
    wcsncpy(errMsg, L"This language doesn't belong to TR-701. Reason: ", 255);
    wcscat(errMsg, message);
    errMsg[255] = L'\0';
    fclose(in_fp);
    nextToken = EOF;
}

/* lookup - a function to lookup reserved keywords and symbols, returning the nextToken */
int lookup(int compareMode) {
    if (compareMode == OPERATOR_MODE) {
        switch (nextChar) {
            case '=':
                addChar();
                getChar();
                if (nextChar == '=') {
                    addChar();
                    nextToken = EQUALITY_OP;
                } else {
                    ungetwc(nextChar, in_fp);
                    nextToken = ASSIGN_OP;
                }
                break;
            case '<':
                addChar();
                getChar();
                if (nextChar == '=') {
                    addChar();
                    nextToken = LE_OP;
                } else {
                    ungetwc(nextChar, in_fp);
                    nextToken = LT_OP;
                }
                break;
            case '>':
                addChar();
                getChar();
                if (nextChar == '=') {
                    addChar();
                    nextToken = GE_OP;
                } else {
                    ungetwc(nextChar, in_fp);
                    nextToken = GT_OP;
                }
                break;
            case '!':
                addChar();
                getChar();
                if (nextChar == '=') {
                    addChar();
                    nextToken = NOT_EQUALITY_OP;
                } else {
                    ungetwc(nextChar, in_fp);
                    nextToken = NOT_OP;
                }
                break;
            case '&':
                addChar();
                getChar();
                if (nextChar == '&') {
                    addChar();
                    nextToken = AND_OP;
                } else {
                    ungetwc(nextChar, in_fp);
                    nextToken = UNREGISTERED_SYMBOL;
                }
                break;
            case '|':
                addChar();
                getChar();
                if (nextChar == '|') {
                    addChar();
                    nextToken = OR_OP;
                } else {
                    ungetwc(nextChar, in_fp);
                    nextToken = UNREGISTERED_SYMBOL;
                }
                break;
            case '+':
                addChar();
                nextToken = ADD_OP;
                break;
            case '-':
                addChar();
                nextToken = SUB_OP;
                break;
            case '*':
                addChar();
                nextToken = MULT_OP;
                break;
            case '/':
                addChar();
                nextToken = DIV_OP;
                break;
            case '^':
                addChar();
                nextToken = POWER_OP;
                break;
            case '%':
                addChar();
                nextToken = MOD_OP;
                break;
            case '(':
                addChar();
                nextToken = LEFT_PAREN;
                break;
            case ')':
                addChar();
                nextToken = RIGHT_PAREN;
                break;
            case '{':
                addChar();
                nextToken = LEFT_CURLY;
                break;
            case '}':
                addChar();
                nextToken = RIGHT_CURLY;
                break;
            case '[':
                addChar();
                nextToken = LEFT_SQUARE;
                break;
            case ']':
                addChar();
                nextToken = RIGHT_SQUARE;
                break;
            case '.':
                addChar();
                nextToken = EOL;
                break;
            case ',':
                addChar();
                nextToken = COMMA;
                break;
            case '\'':
                addChar();
                nextToken = APOSTROPHE;
                break;
            case '$':
                addChar();
                nextToken = COMMENT;
                break;
            default:
                addChar();
                nextToken = UNREGISTERED_SYMBOL;
                break;
        }

    } else if (compareMode == KEYWORD_MODE) {
        if (wcscmp(lexeme, L"if") == 0) {
            nextToken = IF_CODE;
        } else if (wcscmp(lexeme, L"else") == 0) {
            nextToken = ELSE_CODE;
        } else if (wcscmp(lexeme, L"while") == 0) {
            nextToken = WHILE_CODE;
        } else if (wcscmp(lexeme, L"for") == 0) {
            nextToken = FOR_CODE;
        } else if (wcscmp(lexeme, L"return") == 0) {
            nextToken = RETURN_CODE;
        } else if (wcscmp(lexeme, L"break") == 0) {
            nextToken = BREAK_CODE;
        } else if (wcscmp(lexeme, L"continue") == 0) {
            nextToken = CONTINUE_CODE;
        } else if (wcscmp(lexeme, L"true") == 0){
            nextToken = TRUE_VAL;
        } else if (wcscmp(lexeme, L"false") == 0) {
            nextToken = FALSE_VAL;
        } else if (wcscmp(lexeme, L"int") == 0) {
            nextToken = TYPE_INT;
        } else if (wcscmp(lexeme, L"bool") == 0) {
            nextToken = TYPE_BOOL;
        } else if (wcscmp(lexeme, L"char") == 0) {
            nextToken = TYPE_CHAR;
        } else if (wcscmp(lexeme, L"float") == 0) {
            nextToken = TYPE_FLOAT;
        } else if (wcscmp(lexeme, L"double") == 0) {
            nextToken = TYPE_DOUBLE;
        } else {
            nextToken = IDENT;
        }

    } else if (compareMode == TURKISH_LETTER_MODE) {
        switch (nextChar) {
            case L'Ç':
                nextChar = 128;
                return 1;
            case L'ç':
                nextChar = 135;
                return 1;
            case L'ı':
                nextChar = 141;
                return 1;
            case L'İ':
                nextChar = 152;
                return 1;
            case L'ö':
                nextChar = 148;
                return 1;
            case L'Ö':
                nextChar = 153;
                return 1;
            case L'Ü':
                nextChar = 154;
                return 1;
            case L'ü':
                nextChar = 129;
                return 1;
            case L'Ş':
                nextChar = 158;
                return 1;
            case L'ş':
                nextChar = 159;
                return 1;
            case L'Ğ':
                nextChar = 166;
                return 1;
            case L'ğ':
                nextChar = 167;
                return 1;
            default:
                return 0;
        }
    } else {
        printf("Invalid compare mode for lookup function.\n");
        fclose(in_fp);
        return 0;
    }
    return nextToken;
}

/* addChar - a function to add nextChar to lexeme */
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = 0;
    }
    else {
        printf("Lexeme is too long.\n");
        fclose(in_fp);
    }
}

/* getChar - a function to get the next character of input and determine its character class */
void getChar() {
    if ((nextChar = fgetwc(in_fp)) != WEOF) {
        if (isalpha(nextChar) || lookup(TURKISH_LETTER_MODE))
            charClass = LETTER;
        else if (isdigit(nextChar))
            charClass = DIGIT;
        else
            charClass = UNKNOWN;
    }
    else charClass = EOF;
}

/* getNonBlank - a function to call getChar until it returns a non-whitespace character */
void getNonBlank() {
    while (isspace(nextChar))
        getChar();
}

/* lex - a simple lexical analyzer for arithmetic expressions */
int lex() {
    lexLen = 0;
    getNonBlank();
    switch (charClass) {
        case LETTER:
            addChar();
            getChar();
            while (charClass == LETTER || charClass == DIGIT || nextChar == '_') {
                addChar();
                getChar();
            }
            lookup(KEYWORD_MODE);
            break;
        case DIGIT:
            addChar();
            getChar();
            while (charClass == DIGIT) {
                addChar();
                getChar();
            }
            if (nextChar == ',') {
                addChar();
                getChar();
                while (charClass == DIGIT) {
                    addChar();
                    getChar();
                }
                nextToken = FP_LIT;
            }
            else
                nextToken = INT_LIT;
            break;
        case UNKNOWN:
            lookup(OPERATOR_MODE);
            getChar();
            break;
        case EOF:
            nextToken = EOF;
            lexeme[0] = 'E';
            lexeme[1] = 'O';
            lexeme[2] = 'F';
            lexeme[3] = 0;
            break;
    }
    wprintf(L"Next token is: %d, Next lexeme is: %ls\n", nextToken, lexeme);
    return nextToken;
}

/* Funtion program
<program> -> <statementList>
*/
void program() {
    printf("Enter <program>\n");
    statementList();
    if (nextToken != EOF) {
        error(L"Unexpected token after statement list.");
    } else
        printf("Exit <program>\n");
    wprintf(errMsg);
}

/* Function statementList
<statementList> -> {(<statement> '.' | <controlStatement>)}
*/
void statementList() {
    printf("Enter <statementList>\n");
    while (nextToken != EOF && nextToken != RIGHT_CURLY) {
        if (nextToken != IF_CODE && nextToken != WHILE_CODE && nextToken != FOR_CODE) {
            statement();
            if (nextToken == EOL) {
                lex();
            } else {
                error(L"Expected a '.' after a statement.");
            }
        } else {
            controlStatement();
        }
    }
    printf("Exit <statementList>\n");
}

/* Function statement
<statement> -> <boolExpr> | <declStmt> | <assignStmt>
*/
void statement() {
    printf("Enter <statement>\n");
    if (nextToken == TYPE_INT || nextToken == TYPE_BOOL || nextToken == TYPE_CHAR || nextToken == TYPE_FLOAT || nextToken == TYPE_DOUBLE) {
        declStmt();
    } else if (nextToken == IDENT) {
        assignStmt();
    } else if (nextToken == TRUE_VAL || nextToken == FALSE_VAL || nextToken == NOT_OP || nextToken == LEFT_PAREN || nextToken == INT_LIT || nextToken == FP_LIT) {
        boolExpr();
    } else {
        error(L"Illegal statement starting with token.");
    }
    printf("Exit <statement>\n");
}

/* Function controlStatement
<controlStatement> -> <ifStmt> | <whileStmt> | <forStmt>
*/
void controlStatement() {
    printf("Enter <controlStatement>\n");
    switch (nextToken) {
        case IF_CODE:
            ifStmt();
            break;
        case WHILE_CODE:
            whileStmt();
            break;
        case FOR_CODE:
            forStmt();
            break;
    }
    printf("Exit <controlStatement>\n");
}

/* Function expr
<expr> -> <term> {('+' | '-') <term>}
*/
void expr() {
    printf("Enter <expr>\n");
    /* Parse the first term */
    term();
    /* As long as the next token is + or -, get
    the next token and parse the next term */
    while (nextToken == ADD_OP || nextToken == SUB_OP) {
        lex();
        term();
    }
    printf("Exit <expr>\n");
}

/* Function term
<term> -> <factor> {('*' | '/' | '%') <factor>}
*/
void term() {
    printf("Enter <term>\n");
    /* Parse the first factor */
    factor();
    /* As long as the next token is * or /, get the
    next token and parse the next factor */
    while (nextToken == MULT_OP || nextToken == DIV_OP || nextToken == MOD_OP) {
        lex();
        factor();
    }
    printf("Exit <term>\n");
}

/* Function factor
<factor> -> IDENT | INT_LIT | FP_LIT | '(' <expr> ')'
*/
void factor() {
    printf("Enter <factor>\n");
    /* Determine which RHS */
    if (nextToken == IDENT || nextToken == INT_LIT || nextToken == FP_LIT)
    /* Get the next token */
        lex();
    /* If the RHS is (<expr>), call lex to pass over the left parenthesis, call expr, and check for the right parenthesis */
    else {
        if (nextToken == LEFT_PAREN) {
            lex();
            expr();
            if (nextToken == RIGHT_PAREN)
                lex();
            else {
                printf("Where is the right parenthesis?\n");
                fclose(in_fp);
            }
        }
        /* It was not an id, an integer literal, or a left parenthesis */
        else {
            printf("An operand of an expression can only be an identifier, an integer literal, or a left parenthesis.\n");
            fclose(in_fp);
        }
    }
    printf("Exit <factor>\n");
}

/* Function ifStmt
<ifStmt> -> "if" '(' <boolExpr> ')' '{' <statementList> '}' ["else" '{' <statementList> '}']
*/
void ifStmt() {
    printf("Enter <ifStmt>\n");
    /* Check the "if" token */
    if (nextToken != IF_CODE) {
        printf("Expected \"if\" keyword.\n");
        fclose(in_fp);
    } else {
        /* Consume the "if" token */
        lex();
        if (nextToken != LEFT_PAREN) {
            printf("Expected a left parenthesis after \"if\".\n");
            fclose(in_fp);
        } else {
            /* Call lex to get to the next token */
            lex();
            /* Call boolExpr to parse the Boolean expression */
            boolExpr();
            if (nextToken != RIGHT_PAREN) {
                printf("Where is the right parenthesis?\n");
                fclose(in_fp);
            } else {
                /* Consume the right parenthesis */
                lex();
                if (nextToken != LEFT_CURLY) {
                    printf("Expected a left curly brace after the condition in if statement.\n");
                    fclose(in_fp);
                } else {
                    /* Consume the left curly brace */
                    lex();
                    /* Call statementList to parse the then clause */
                    statementList();
                    if (nextToken != RIGHT_CURLY) {
                        printf("Where is the right curly brace?\n");
                        fclose(in_fp);
                    } else {
                        /* Consume the right curly brace */
                        lex();
                        /* If an else is next along with a left curly, parse the else clause */
                        if (nextToken == ELSE_CODE) {
                            /* Consume the "else" token */
                            lex();
                            if (nextToken != LEFT_CURLY) {
                                printf("Expected a left curly brace after \"else\".\n");
                                fclose(in_fp);
                            } else {
                                /* Consume the left curly brace */
                                lex();
                                /* Call statementList to parse the else clause */
                                statementList();
                            }
                        }
                    }
                }
            }
        }
    }
    printf("Exit <ifStmt>\n");
}

/* Function boolExpr
<boolExpr> -> <boolTerm> {"||" <boolTerm>}
*/
void boolExpr() {
    printf("Enter <boolExpr>\n");
    boolTerm();
    while (nextToken == OR_OP){
        /* Consume the "||" token */
        lex();
        /* Call boolTerm to parse the next part of the expression */
        boolTerm();
    }
    printf("Exit <boolExpr>\n");
}

/* Function boolTerm
<boolTerm> -> <boolEquality> {"&&" <boolEquality>}
*/
void boolTerm() {
    printf("Enter <boolTerm>\n");
    boolEquality();
    while (nextToken == AND_OP) {
        /* Consume the "&&" token */
        lex();
        /* Call boolEquality to parse the next part of the expression */
        boolEquality();
    }
    printf("Exit <boolTerm>\n");
}

/* Function boolEquality
<boolEquality> -> <boolRelational> {("==" | "!=") <boolRelational>}
*/
void boolEquality() {
    printf("Enter <boolEquality>\n");
    boolRelational();
    while (nextToken == EQUALITY_OP || nextToken == NOT_EQUALITY_OP) {
        /* Consume the "==" or "!=" token */
        lex();
        /* Call boolRelational to parse the next part of the expression */
        boolRelational();
    }
    printf("Exit <boolEquality>\n");
}

/* Function boolRelational
<boolRelational> -> <boolFactor> {("<" | ">" | "<=" | ">=") <boolFactor>}
*/
void boolRelational() {
    printf("Enter <boolRelational>\n");
    boolFactor();
    while (nextToken == GT_OP || nextToken == LT_OP || nextToken == GE_OP || nextToken == LE_OP) {
        /* Consume the comparison operator */
        lex();
        /* Call boolFactor to parse the next part of the expression */
        boolFactor();
    }
    printf("Exit <boolRelational>\n");
}

/* Function boolFactor
<boolFactor> -> "true" | "false" | <boolNot> | <expr> | '(' <boolExpr> ')'
*/
void boolFactor() {
    printf("Enter <boolFactor>\n");
    /* If the next token is a NOT operator, call boolNot */
    if (nextToken == NOT_OP)
        boolNot();
    // Check the next token to determine the type of boolean factor
    if (nextToken == TRUE_VAL || nextToken == FALSE_VAL) {
        // Consume the boolean literal
        lex();
    } else if (nextToken == LEFT_PAREN) {
        // Consume the left parenthesis
        lex();
        // Parse the boolean expression inside the parentheses
        boolExpr();
        // Check for the right parenthesis
        if (nextToken != RIGHT_PAREN) {
            printf("Where is the right parenthesis?\n");
            fclose(in_fp);
        } else {
            // Consume the right parenthesis
            lex();
        }
    } else
        expr();
    printf("Exit <boolFactor>\n");
}

/* Function boolNot
<boolNot> -> '!' <boolFactor>
*/
void boolNot() {
    printf("Enter <boolNot>\n");
    /* Consume the NOT operator */
    lex();
    /* Call boolFactor to parse the factor after NOT */
    boolFactor();
    printf("Exit <boolNot>\n");
}

/* Function declStmt
<declStmt> -> ("int" | "float" | "double") IDENT ['=' <expr>]
                | "char" IDENT ['=' <charLit>]
                | "bool" IDENT ['=' <boolExpr>]
*/
void declStmt() {
    printf("Enter <declStmt>\n");
    if (nextToken == TYPE_INT || nextToken == TYPE_FLOAT || nextToken == TYPE_DOUBLE) {
        /* Consume the type token */
        lex();
        if (nextToken != IDENT) {
            printf("Expected an identifier after type declaration.\n");
            fclose(in_fp);
        } else {
            /* Consume the identifier */
            lex();
            /* Check for an assignment operator */
            if (nextToken == ASSIGN_OP) {
                /* Get the next token */
                lex();
                /* Call expr to parse the expression */
                expr();
            } else if (nextToken != EOL) {
                printf("Expected an assignment operator or end of line after variable declaration.\n");
                fclose(in_fp);
            }
        }
    } else if (nextToken == TYPE_CHAR) {
        /* Consume the "char" token */
        lex();
        if (nextToken != IDENT) {
            printf("Expected an identifier after 'char'.\n");
            fclose(in_fp);
        } else {
            /* Consume the identifier */
            lex();
            /* Check for an assignment operator */
            if (nextToken == ASSIGN_OP) {
                /* Get the next token */
                lex();
                /* Check for a character literal */
                charLit();
            } else if (nextToken != EOL) {
                printf("Expected an assignment operator or end of line after variable declaration.\n");
                fclose(in_fp);
            }
        }
    } else if (nextToken == TYPE_BOOL) {
        /* Consume the "bool" token */
        lex();
        if (nextToken != IDENT) {
            printf("Expected an identifier after 'bool'.\n");
            fclose(in_fp);
        } else {
            /* Consume the identifier */
            lex();
            /* Check for an assignment operator */
            if (nextToken == ASSIGN_OP) {
                /* Get the next token */
                lex();
                /* Call boolExpr to parse the boolean expression */
                boolExpr();
            } else if (nextToken != EOL) {
                printf("Expected an assignment operator or end of line after variable declaration.\n");
                fclose(in_fp);
            }
        }
    }
    else {
        printf("Invalid type declaration: %d\n", nextToken);
        fclose(in_fp);
    }
    printf("Exit <declStmt>\n");
}

/* Function charLit
<charLit> -> ' CHAR '
*/
void charLit() {
    printf("Enter <charLit>\n");
    /* Check for the opening single quote */
    if (nextToken != APOSTROPHE) {
        printf("Expected a single quote before character literal.\n");
        fclose(in_fp);
    } else {
        /* Consume the opening single quote */
        lex();
        /* Check if the length is valid  */
        if (lexLen == 1) {
            /* Consume the character */
            lex();
            /* Check for the closing single quote */
            if (nextToken != APOSTROPHE) {
                printf("Expected a single quote after character literal.\n");
                fclose(in_fp);
            } else {
                /* Consume the closing single quote */
                lex();
            }
        } else {
            printf("Character literal must be a single character.\n");
            fclose(in_fp);
        }
    }
    printf("Exit <charLit>\n");
}

/* Function assignStmt
<assignStmt> -> IDENT '=' (<expr> | <charLit> | <boolExpr>)
*/
void assignStmt() {
    printf("Enter <assignStmt>\n");
    if (nextToken != IDENT) {
        printf("Expected an identifier for assignment.\n");
        fclose(in_fp);
    } else {
        /* Consume the identifier */
        lex();
        /* Check for the assignment operator */
        if (nextToken != ASSIGN_OP) {
            printf("Expected an assignment operator after identifier.\n");
            fclose(in_fp);
        } else {
            /* Consume the assignment operator */
            lex();
            /* Check the next token to determine the type of assignment */
            if (nextToken == INT_LIT || nextToken == FP_LIT || nextToken == IDENT || nextToken == LEFT_PAREN ||
                nextToken == TRUE_VAL || nextToken == FALSE_VAL || nextToken == NOT_OP) {
                /* Call boolExpr since it contains both expr and boolExpr on a non-semantic level */
                boolExpr();
            } else if (nextToken == APOSTROPHE) {
                /* Call charLit to parse the character literal */
                charLit();
            } else {
                printf("Invalid assignment value: %d\n", nextToken);
                fclose(in_fp);
                nextToken = EOF;
            }
        }
    }
    printf("Exit <assignStmt>\n");
}

/* Function whileStmt
<whileStmt> -> "while" '(' <boolExpr> ')' '{' <statementList> '}'
*/
void whileStmt() {
    printf("Enter <whileStmt>\n");
    /* Check the "while" token */
    if (nextToken != WHILE_CODE) {
        printf("Expected 'while' keyword.\n");
        fclose(in_fp);
    } else {
        /* Consume the "while" token */
        lex();
        /* Check for the left parenthesis */
        if (nextToken != LEFT_PAREN) {
            printf("Expected a left parenthesis after \"while\".\n");
            fclose(in_fp);
        } else {
            /* Consume the left parenthesis */
            lex();
            /* Call boolExpr to parse the Boolean expression */
            boolExpr();
            /* Check for the right parenthesis */
            if (nextToken != RIGHT_PAREN) {
                printf("Where is the right parenthesis?\n");
                fclose(in_fp);
            } else {
                /* Consume the right parenthesis */
                lex();
                /* Check for the left curly brace */
                if (nextToken != LEFT_CURLY) {
                    printf("Expected a left curly brace after while loop condition.\n");
                    fclose(in_fp);
                } else {
                    /* Consume the left curly brace */
                    lex();
                    /* Call statementList to parse the loop body */
                    statementList();
                    /* Check for the right curly brace */
                    if (nextToken != RIGHT_CURLY) {
                        printf("Where is the right curly brace?\n");
                        fclose(in_fp);
                    } else {
                        /* Consume the right curly brace */
                        lex();
                    }
                }
            }
        }
    }
    printf("Exit <whileStmt>\n");
}

/* Function forStmt
<forStmt> -> "for" '(' <assignStmt> '.' <boolExpr> '.' <assignStmt> ')' '{' <statementList> '}'
*/
void forStmt() {
    printf("Enter <forStmt>\n");
    /* Check the "for" token */
    if (nextToken != FOR_CODE) {
        printf("Expected \"for\" keyword.\n");
        fclose(in_fp);
    } else {
        /* Consume the "for" token */
        lex();
        /* Check for the left parenthesis */
        if (nextToken != LEFT_PAREN) {
            printf("Expected a left parenthesis after \"for\".\n");
            fclose(in_fp);
        } else {
            /* Consume the left parenthesis */
            lex();
            /* Parse the first assignment statement */
            assignStmt();
            /* If assignStmt raised error, exit <forStmt> */
            if (nextToken == EOF) {
                printf("Exit <forStmt>\n");
                return;
            }
            /* Check for dot */
            if (nextToken != EOL) {
                printf("Expected '.' after the first assignment in for loop.\n");
                fclose(in_fp);
            } else {
                /* Consume the dot */
                lex();
                /* Call boolExpr to parse the loop condition */
                boolExpr();
                /* Check for the second dot */
                if (nextToken != EOL) {
                    printf("Expected a '.' after the boolean expression in for loop.\n");
                    fclose(in_fp);
                } else {
                    /* Consume the second dot */
                    lex();
                    /* Parse the second assignment statement */
                    assignStmt();
                    /* Check for the right parenthesis */
                    if (nextToken != RIGHT_PAREN) {
                        printf("Where is the right parenthesis?\n");
                        fclose(in_fp);
                    } else {
                        /* Consume the right parenthesis */
                        lex();
                        /* Check for the left curly brace */
                        if (nextToken != LEFT_CURLY) {
                            printf("Expected a left curly brace after for loop condition.\n");
                            fclose(in_fp);
                        } else {
                            /* Consume the left curly brace */
                            lex();
                            /* Call statementList to parse the loop body */
                            statementList();
                            /* Check for the right curly brace */
                            if (nextToken != RIGHT_CURLY) {
                                printf("Where is the right curly brace?\n");
                                fclose(in_fp);
                            } else {
                                /* Consume the right curly brace */
                                lex();
                            }
                        }
                    }
                }
            }
        }
    }
    printf("Exit <forStmt>\n");
}

// TODO: Ambiguity in between <boolExpr> and <expr> is partially resolved technically by <boolExpr> containing <expr> inside in it, and only <boolExpr> being called in <statement>. But is it possible to have a case where <expr> is called directly in <statement>?
// TODO: Convert all EBNFs to BNF (uncertain if it is needed for the project) (This might cause some changes in the code...)
// TODO: Add a function to handle comments
// TODO: Add a function to handle string literals
// TODO: Need a universal error handling function that can be used in all functions, also telling if the language belongs to ours or not at the end.