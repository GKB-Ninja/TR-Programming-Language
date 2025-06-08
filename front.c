/* front.c - a lexical analyzer system for simple arithmetic expressions */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>


/***  Global Declarations  ***/

/* Variables */
int charClass;
wchar_t lexeme [100];
wchar_t nextChar;
int lexLen;
int token;
char nextToken;
FILE *in_fp;

/* Function declarations */
void addChar();
void getChar();
void getNonBlank();
int lex();
int lookup(int compareMode);

void expr();
void factor();
void term();
void error();

void boolExpr();
void boolExprHigher();
void boolTerm();
void boolTermHigher();
void boolFactor();
void boolNot();

void ifStmt();
void whileStmt();
void forStmt();
void returnStmt();
void declStmt();
void assignStmt();
void statement();

/* Character classes */
#define DIGIT 0
#define LETTER 1
#define UNKNOWN 2

/* Token codes */
#define INT_LIT 10
#define IDENT 11
#define ASSIGN_OP 12
#define EQUALITY_OP 13
#define NOT_EQUALITY_OP 14
#define LE_OP 15
#define GE_OP 16
#define LT_OP 17
#define GT_OP 18
#define NOT_OP 19
#define AND_OP 20
#define OR_OP 21
#define ADD_OP 22
#define SUB_OP 23
#define MULT_OP 24
#define DIV_OP 25
#define POWER_OP 26
#define MOD_OP 27
#define COMMA 28
#define IF_CODE 30
#define ELSE_CODE 31
#define WHILE_CODE 32
#define FOR_CODE 33
#define CONTINUE_CODE 34
#define BREAK_CODE 35
#define LEFT_PAREN 40
#define RIGHT_PAREN 41
#define LEFT_CURLY 42
#define RIGHT_CURLY 43
#define LEFT_SQUARE 44
#define RIGHT_SQUARE 45
#define RETURN_CODE 50
#define TYPE_INT 60
#define TYPE_BOOL 61
#define TYPE_CHAR 62
#define TYPE_FLOAT 63
#define TYPE_DOUBLE 64
#define TRUE_VAL 70
#define FALSE_VAL 71
#define EOL 90
#define UNREGISTERED_SYMBOL 99

/* Extras */
#define TURKISH_LETTER 3
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
        do {
            lex();
            statement();
        } while (nextToken != EOF);
    }
}
/************************************************************************************/

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
            case ',':
                addChar();
                nextToken = COMMA;
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
                return TURKISH_LETTER;
            case L'ç':
                nextChar = 135;
                return TURKISH_LETTER;
            case L'ı':
                nextChar = 141;
                return TURKISH_LETTER;
            case L'İ':
                nextChar = 152;
                return TURKISH_LETTER;
            case L'ö':
                nextChar = 148;
                return TURKISH_LETTER;
            case L'Ö':
                nextChar = 153;
                return TURKISH_LETTER;
            case L'Ü':
                nextChar = 154;
                return TURKISH_LETTER;
            case L'ü':
                nextChar = 129;
                return TURKISH_LETTER;
            case L'Ş':
                nextChar = 158;
                return TURKISH_LETTER;
            case L'ş':
                nextChar = 159;
                return TURKISH_LETTER;
            case L'Ğ':
                nextChar = 166;
                return TURKISH_LETTER;
            case L'ğ':
                nextChar = 167;
                return TURKISH_LETTER;
            default: return 0;
        }
    } else {
        printf("Invalid compare mode for lookup function.\n");
        return -1;
    }
    return nextToken;
}

/* addChar - a function to add nextChar to lexeme */
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = 0;
    }
    else
        printf("Lexeme is too long.\n");
}

/* getChar - a function to get the next character of input and determine its character class */
void getChar() {
    if ((nextChar = fgetwc(in_fp)) != WEOF) {
        if (isalpha(nextChar) || lookup(TURKISH_LETTER_MODE)) charClass = LETTER;
        else if (isdigit(nextChar)) charClass = DIGIT;
        else charClass = UNKNOWN;
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
            while (charClass == LETTER || charClass == DIGIT) {
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

/*
 <statement> -> {(<expr> | <ifStmt> | <boolExp>)}
 */
void statement() {
    if (nextToken == INT_LIT || nextToken == IDENT || nextToken == LEFT_PAREN || nextToken == RIGHT_PAREN)
    {
        expr();
    }
    else if (nextToken == IF_CODE) {
        ifStmt();
    }
    else
        printf("Invalid statement.\n");
}

/* expr
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

/* term
<term> -> <factor> {('*' | '/') <factor>)}
*/
void term() {
    printf("Enter <term>\n");
    /* Parse the first factor */
    factor();
    /* As long as the next token is * or /, get the
    next token and parse the next factor */
    while (nextToken == MULT_OP || nextToken == DIV_OP) {
        lex();
        factor();
    }
    printf("Exit <term>\n");
}

/* factor
<factor> -> ident | int_constant | '(' <expr> ')'
*/
void factor() {
    printf("Enter <factor>\n");
    /* Determine which RHS */
    if (nextToken == IDENT || nextToken == INT_LIT)
    /* Get the next token */
        lex();
    /* If the RHS is (<expr>), call lex to pass over the
    left parenthesis, call expr, and check for the right
    parenthesis */
    else {
        if (nextToken == LEFT_PAREN) {
            lex();
            expr();
            if (nextToken == RIGHT_PAREN)
                lex();
            else
                printf("Where is the right parenthesis?\n");
        } /* End of if (nextToken == ... */
        /* It was not an id, an integer literal, or a left
        parenthesis */
        else
            printf("Where is the id, int_constant, or left parenthesis?\n");
    } /* End of else */
    printf("Exit <factor>\n");
}

/* Function ifStmt
<ifStmt> -> "if" (<boolExpr>) <statement>
[else <statement>]
*/
void ifStmt() {
    printf("Enter <ifStmt>\n");
    /* Be sure the first token is 'if' */
    if (nextToken != IF_CODE)
        printf("Where is the if-code?\n");
    else {
        /* Call lex to get to the next token */
        lex();
        /* Check for the left parenthesis */
        if (nextToken != LEFT_PAREN)
            printf("Where is the left parenthesis?\n");
        else {
            /* Call lex to get to the next token */
            lex();
            /* Call boolExpr to parse the Boolean expression */
            boolExpr();
            /* Check for the right parenthesis */
            if (nextToken != RIGHT_PAREN)
                printf("Where is the right parenthesis?\n");
            else {
                /* Call statement to parse the then clause */
                statement();
                /* If an else is next, parse the else clause */
                if (nextToken == ELSE_CODE) {
                    /* Call lex to get over the else */
                    lex();
                    statement();
                } /* end of if (nextToken == ELSE_CODE ... */
            } /* end of else of if (nextToken != RIGHT ... */
        } /* end of else of if (nextToken != LEFT ... */
    } /* end of else of if (nextToken != IF_CODE ... */
    printf("Exit <ifStmt>\n");
} /* end of ifStmt */

/* Function boolExpr
<boolExpr> -> <boolExprHigher> {"||" <boolExprHigher>}
*/
void boolExpr() {
    printf("Enter <boolExpr>\n");
    boolExprHigher();
    while (nextToken == OR_OP){
        boolTermHigher();
    }
    printf("Exit <boolExpr>\n");
}

/* Function boolExprHigher
<boolExprHigher> -> <boolTerm> {"&&" <boolTerm>}
*/
void boolExprHigher() {
    printf("Enter <boolExprHigher>\n");
    boolTerm();
    while (nextToken == AND_OP) {
        boolTerm();
    }
    printf("Exit <boolExprHigher>\n");
}

/* Function boolTerm
<boolTerm> -> <boolTermHigher> {("==" | "!=") <boolTermHigher>}
*/
void boolTerm() {
    printf("Enter <boolTerm>\n");
    boolTermHigher();
    while (nextToken == EQUALITY_OP || nextToken == NOT_EQUALITY_OP) {
        boolTermHigher();
    }
    printf("Exit <boolTerm>\n");
}

/* Function boolTermHigher
<boolTermHigher> -> <factor> {("<" | ">" | "<=" | ">=") <factor>}
*/
void boolTermHigher() {
    printf("Enter <boolTermHigher>\n");
    boolFactor();
    while (nextToken == GT_OP || nextToken == LT_OP || nextToken == GE_OP || nextToken == LE_OP) {
        boolTerm();
    }
    printf("Exit <boolTermHigher>\n");
}

/* Function boolFactor
<boolFactor> -> <boolNot> | <expr> | '(' <boolExpr> ')'
*/
void boolFactor() {
    printf("Enter <boolFactor>\n");
    if (nextToken == NOT_OP)
        boolNot();
    printf("Exit <boolFactor>\n");
}

/* Function boolNot
<boolNot> -> '!' <boolFactor>
*/
void boolNot() {
    boolFactor();
// TODO: boolNot's BNF isn't looking great.
}

/* Function declStmt
 <declStmt> -> <dataType> <identifier> ['=' <expr>]
 */
void declStmt() {
    printf("Enter <declStmt>\n");

    printf("Exit <declStmt>\n");
}

void assignStmt() {
    printf("Enter <assignStmt>\n");

    printf("Exit <assignStmt>\n");
}

// TODO: I suggest reconstruction of the parse tree. Ambiguity in seperating comparison, logical and arithmetic.