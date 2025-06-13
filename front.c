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

/* Functions */
void addChar();
void getChar();
void getNonBlank();
int lex();
int lookup(int compareMode);
void error(const wchar_t *message);

void program();
void statementList();
void statement();
void controlStatement();

void expr();
void factor();
void term();
void power();

void boolExpr();
void boolOr();
void boolAnd();
void boolEq();
void boolRel();
void boolArithExpr();
void boolArithTerm();
void boolArithPower();
void boolArithNot();
void boolArithFactor();

void charLit();
void stringLit();

void ifStmt();
void whileStmt();
void forStmt();
void declStmt();
void assignStmt();

/* Character classes */
#define DIGIT 0
#define LETTER 1
#define UNKNOWN 2
#define COMMENT 3

/* Token codes */
#define INT_LIT 10
#define FP_LIT 11
#define IDENT 12
#define TYPE_INT 13
#define TYPE_FLOAT 14
#define TYPE_DOUBLE 15
#define TYPE_CHAR 16
#define TYPE_STRING 17
#define TYPE_BOOL 18
#define TRUE_VAL 19
#define FALSE_VAL 20
#define ASSIGN_OP 30
#define EQUALITY_OP 31
#define NOT_EQUALITY_OP 32
#define LE_OP 33
#define GE_OP 34
#define LT_OP 35
#define GT_OP 36
#define NOT_OP 37
#define AND_OP 38
#define OR_OP 39
#define ADD_OP 40
#define SUB_OP 41
#define MULT_OP 42
#define DIV_OP 43
#define POWER_OP 44
#define MOD_OP 45
#define IF_CODE 60
#define ELSE_CODE 61
#define WHILE_CODE 62
#define FOR_CODE 63
#define BREAK_CODE 64
#define CONTINUE_CODE 65
#define LEFT_PAREN 80
#define RIGHT_PAREN 81
#define LEFT_CURLY 82
#define RIGHT_CURLY 83
#define LEFT_SQUARE 84
#define RIGHT_SQUARE 85
#define EOS 90
#define COMMA 91
#define APOSTROPHE 92
#define QUOTE 93
#define UNDERSCORE 94
#define COMMENT_SYMB 95
#define UNREGISTERED_SYMBOL 99

/* Extras */
#define TURKISH_LETTER_MODE 4
#define OPERATOR_MODE 5
#define KEYWORD_MODE 6

/************************************************************************************/

/* main driver */
int main() {
    setlocale(LC_ALL, "");
    if ((in_fp = fopen("front.in", "rb")) == NULL) {
        perror("front.in is not in the executable's directory or cannot be opened.");
    } else {
        unsigned char bom[2];
        if (fread(bom, 1, 2, in_fp) != 2 || bom[0] != 0xFF || bom[1] != 0xFE) {
            printf("front.in is not in UTF-16LE format.\n");
            fclose(in_fp);
            return 1;
        }

        getChar();
        lex();
        program();

        fclose(in_fp);
    }
    return 0;
}

/************************************************************************************/

/* error - a universal error handling function */
void error(const wchar_t *message) {
    static int errorRaised = 0; // Flag to track if an error has already been raised
    if (!errorRaised) {
        errorRaised = 1; // Set the flag to indicate an error has been raised
        wcsncpy(errMsg, L"This language doesn't belong to TR-701.\nReason: ", 255);
        wcscat(errMsg, message);
        errMsg[255] = L'\0';
        fclose(in_fp);
        nextToken = EOF;
    }
}

/* lookup - a function to lookup reserved keywords and symbols, returning the nextToken */
int lookup(int compareMode) {
    if (compareMode == OPERATOR_MODE) {
        switch (nextChar) {
            case '=':
                addChar();
                getChar();
                if (nextChar == '?') {
                    addChar();
                    nextToken = EQUALITY_OP;
                } else {
                    ungetwc(nextChar, in_fp);
                    nextToken = UNREGISTERED_SYMBOL;
                }
                break;
            case '<':
                /* "<" is LT, "<=" is LE, "<<<" is ASSIGN_OP */
                addChar();
                getChar();
                if (nextChar == '=') {
                    addChar();
                    nextToken = LE_OP;
                } else if (nextChar == '<') {
                    wchar_t temp = nextChar;
                    addChar();
                    getChar();
                    if (nextChar == '<') {
                        addChar();
                        nextToken = ASSIGN_OP;
                    } else {
                        ungetwc(nextChar, in_fp);
                        ungetwc(temp, in_fp);
                        lexeme[1] = 0;
                        nextToken = LT_OP;
                    }
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
                if (nextChar == '?') {
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
                nextToken = EOS;
                break;
            case ',':
                addChar();
                nextToken = COMMA;
                break;
            case '\'':
                addChar();
                nextToken = APOSTROPHE;
                break;
            case '"':
                addChar();
                nextToken = QUOTE;
                break;
            case '_':
                addChar();
                nextToken = UNDERSCORE;
                break;
            case '$':
                addChar();
                nextToken = COMMENT_SYMB;
                break;
            default:
                addChar();
                nextToken = UNREGISTERED_SYMBOL;
                break;
        }

    } else if (compareMode == KEYWORD_MODE) {
        if (wcscmp(lexeme, L"tam") == 0) {
            nextToken = TYPE_INT;
        } else if (wcscmp(lexeme, L"küsurat") == 0) {
            nextToken = TYPE_FLOAT;
        } else if (wcscmp(lexeme, L"dev") == 0) {
            nextToken = TYPE_DOUBLE;
        } else if (wcscmp(lexeme, L"hane") == 0) {
            nextToken = TYPE_CHAR;
        } else if (wcscmp(lexeme, L"tümce") == 0) {
            nextToken = TYPE_STRING;
        } else if (wcscmp(lexeme, L"mantık") == 0) {
            nextToken = TYPE_BOOL;
        } else if (wcscmp(lexeme, L"doğru") == 0) {
            nextToken = TRUE_VAL;
        } else if (wcscmp(lexeme, L"yanlış") == 0) {
            nextToken = FALSE_VAL;
        } else if (wcscmp(lexeme, L"madem") == 0) {
            nextToken = IF_CODE;
        } else if (wcscmp(lexeme, L"şayet") == 0) {
            nextToken = ELSE_CODE;
        } else if (wcscmp(lexeme, L"iken") == 0) {
            nextToken = WHILE_CODE;
        } else if (wcscmp(lexeme, L"sayaç") == 0) {
            nextToken = FOR_CODE;
        } else if (wcscmp(lexeme, L"çık") == 0) {
            nextToken = BREAK_CODE;
        } else if (wcscmp(lexeme, L"atla") == 0) {
            nextToken = CONTINUE_CODE;
        } else {
            nextToken = IDENT;
        }

    } else if (compareMode == TURKISH_LETTER_MODE) {
        switch (nextChar) {
            case L'Ç':
                return 1;
            case L'ç':
                return 1;
            case L'ı':
                return 1;
            case L'İ':
                return 1;
            case L'ö':
                return 1;
            case L'Ö':
                return 1;
            case L'Ü':
                return 1;
            case L'ü':
                return 1;
            case L'Ş':
                return 1;
            case L'ş':
                return 1;
            case L'Ğ':
                return 1;
            case L'ğ':
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
    unsigned char bytes[2];

    if (fread(bytes, 1, 2, in_fp) == 2) {
        nextChar = bytes[1] << 8 | bytes[0];
    } else {
        nextChar = WEOF;
    }

    if (nextChar != WEOF) {
        if (isalpha(nextChar) || lookup(TURKISH_LETTER_MODE))
            charClass = LETTER;
        else if (isdigit(nextChar))
            charClass = DIGIT;
        else if (nextChar == L'$')
            charClass = COMMENT;
        else
            charClass = UNKNOWN;
    } else {
        charClass = EOF;
    }
}

/* getNonBlank - a function to call getChar until it returns a non-whitespace character */
void getNonBlank() {
    while (iswspace(nextChar))
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
        case COMMENT:
            do {
                getChar();
            } while (charClass != COMMENT && charClass != EOF);
            if (charClass == EOF) {
                error(L"Comments must be opened and closed with '$'.");
            } else {
                /* Skip the closing comment symbol '$' */
                getChar();
                /* Continue lexing after the comment*/
                return lex();
            }
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
        error(L"Wrong use of closing curly brace. Expected EOF.");
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
            if (nextToken == EOS) {
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
<statement> -> "atla" | "çık" | <declStmt> | <assignStmt>
*/
void statement() {
    printf("Enter <statement>\n");
    if (nextToken == CONTINUE_CODE) {
        lex();
    } else if (nextToken == BREAK_CODE) {
        lex();
    } else if (nextToken == TYPE_INT || nextToken == TYPE_BOOL || nextToken == TYPE_CHAR || nextToken == TYPE_FLOAT ||
               nextToken == TYPE_DOUBLE || nextToken == TYPE_STRING) {
        declStmt();
    } else if (nextToken == IDENT) {
        assignStmt();
    } else if (nextToken == TRUE_VAL || nextToken == FALSE_VAL || nextToken == NOT_OP || nextToken == LEFT_PAREN ||
               nextToken == INT_LIT || nextToken == FP_LIT) {
        error(L"Expressions are not allowed as standalone statements. Use them in control statements or assignments.");
    } else {
        error(L"Illegal statement.");
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
<expr> -> <term> {("+" | "-") <term>}
*/
void expr() {
    printf("Enter <expr>\n");
    term();
    while (nextToken == ADD_OP || nextToken == SUB_OP) {
        lex();
        term();
    }
    printf("Exit <expr>\n");
}

/* Function term
<term> -> <power> {("*" | "/" | "%") <power>}
*/
void term() {
    printf("Enter <term>\n");
    power();
    while (nextToken == MULT_OP || nextToken == DIV_OP || nextToken == MOD_OP) {
        lex();
        power();
    }
    printf("Exit <term>\n");
}

/* Function power
<power> -> <factor> "^" <power> | <factor>
*/
void power() {
    printf("Enter <power>\n");
    factor();
    if (nextToken == POWER_OP) {
        lex();
        power();
    }
    printf("Exit <power>\n");
}

/* Function factor
<factor> -> IDENT | INT_LIT | FP_LIT | "(" <expr> ")"
*/
void factor() {
    printf("Enter <factor>\n");
    if (nextToken == IDENT || nextToken == INT_LIT || nextToken == FP_LIT) {
        lex();
    } else if (nextToken == LEFT_PAREN) {
        lex();
        expr();
        if (nextToken == RIGHT_PAREN) {
            lex();
        } else {
            error(L"Expected a right parenthesis after expression.");
        }
    } else {
        error(L"Invalid arithmetic factor. Expected IDENT, INT_LIT, FP_LIT, or '('");
    }
    printf("Exit <factor>\n");
}

/* Function ifStmt
<ifStmt> -> "madem" "(" <boolExpr> ")" "{" <statementList> "}" ["şayet" "{" <statementList> "}"]
*/
void ifStmt() {
    printf("Enter <ifStmt>\n");
    if (nextToken != IF_CODE) {
        error(L"Expected \"if\" keyword.");
    } else {
        lex();
        if (nextToken != LEFT_PAREN) {
            error(L"Expected a left parenthesis after \"if\".");
        } else {
            lex();
            boolExpr();
            if (nextToken != RIGHT_PAREN) {
                error(L"Expected a right parenthesis after if condition.");
            } else {
                lex();
                if (nextToken != LEFT_CURLY) {
                    error(L"Expected a left curly brace after condition in if statement.");
                } else {
                    lex();
                    statementList();
                    if (nextToken != RIGHT_CURLY) {
                        error(L"Expected a right curly brace to close \"if\" statement.");
                    } else {
                        lex();
                        if (nextToken == ELSE_CODE) {
                            lex();
                            if (nextToken != LEFT_CURLY) {
                                error(L"Expected a left curly brace after \"else\".");
                            } else {
                                lex();
                                statementList();
                                if (nextToken != RIGHT_CURLY) {
                                    error(L"Expected a right curly brace to close else clause.");
                                } else {
                                    lex();
                                }
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
<boolExpr> -> <boolOr>
*/
void boolExpr() {
    printf("Enter <boolExpr>\n");
    boolOr();
    printf("Exit <boolExpr>\n");
}

/* Function boolOr
<boolOr> -> <boolAnd> { "||" <boolAnd> }
*/
void boolOr() {
    printf("Enter <boolOr>\n");
    boolAnd();
    while (nextToken == OR_OP) {
        lex();
        boolAnd();
    }
    printf("Exit <boolOr>\n");
}

/* Function boolAnd
<boolAnd> -> <boolEq> { "&&" <boolEq> }
*/
void boolAnd() {
    printf("Enter <boolAnd>\n");
    boolEq();
    while (nextToken == AND_OP) {
        lex();
        boolEq();
    }
    printf("Exit <boolAnd>\n");
}

/* Function boolEq
<boolEq> -> <boolRel> { ("=?" | "!?") <boolRel> }
*/
void boolEq() {
    printf("Enter <boolEq>\n");
    boolRel();
    while (nextToken == EQUALITY_OP || nextToken == NOT_EQUALITY_OP) {
        lex();
        boolRel();
    }
    if (nextToken == LT_OP || nextToken == LE_OP || nextToken == GT_OP || nextToken == GE_OP || nextToken == ADD_OP
        || nextToken == SUB_OP || nextToken == MULT_OP || nextToken == DIV_OP || nextToken == POWER_OP || nextToken == MOD_OP) {
        error(L"A boolean value cannot be compared or operated with arithmetic operators.");
    }
    printf("Exit <boolEq>\n");
}

/* Function boolRel
<boolRel> -> "doğru" | "yanlış" | <boolArithExpr> { ("<" | "<=" | ">" | ">=") <boolArithExpr> }
*/
void boolRel() {
    printf("Enter <boolRel>\n");
    if (nextToken == TRUE_VAL || nextToken == FALSE_VAL) {
        lex();
    } else {
        boolArithExpr();
        while (nextToken == LT_OP || nextToken == LE_OP || nextToken == GT_OP || nextToken == GE_OP) {
            lex();
            boolArithExpr();
        }
    }
    printf("Exit <boolRel>\n");
}

/* Function boolArithExpr
<boolArithExpr> -> <boolArithTerm> { ("+" | "-") <boolArithTerm> }
*/
void boolArithExpr() {
    printf("Enter <boolArithExpr>\n");
    boolArithTerm();
    while (nextToken == ADD_OP || nextToken == SUB_OP) {
        lex();
        boolArithTerm();
    }
    printf("Exit <boolArithExpr>\n");
}

/* Function boolArithTerm
<boolArithTerm> -> <boolArithPower> { ("*" | "/" | "%") <boolArithPower> }
*/
void boolArithTerm() {
    printf("Enter <boolArithTerm>\n");
    boolArithPower();
    while (nextToken == MULT_OP || nextToken == DIV_OP || nextToken == MOD_OP) {
        lex();
        boolArithPower();
    }
    printf("Exit <boolArithTerm>\n");
}

/* Function boolArithPower
<boolArithPower> -> <boolArithNot> "^" <boolArithPower> | <boolArithPower>
*/
void boolArithPower() {
    printf("Enter <boolArithPower>\n");
    boolArithNot();
    if (nextToken == POWER_OP) {
        lex();
        boolArithPower();
    }
    printf("Exit <boolArithPower>\n");
}

/* Function boolArithNot
<boolArithNot> -> "!" <boolArithNot> | <boolArithFactor>
*/
void boolArithNot() {
    printf("Enter <boolArithNot>\n");
    if (nextToken == NOT_OP) {
        lex();
        boolArithNot();
    } else {
        boolArithFactor();
    }
    printf("Exit <boolArithNot>\n");
}

/* Function boolArithFactor
<boolArithFactor> -> IDENT | INT_LIT | FP_LIT | "(" <boolExpr> ")"
*/
void boolArithFactor() {
    printf("Enter <boolArithFactor>\n");
    if (nextToken == IDENT || nextToken == INT_LIT || nextToken == FP_LIT) {
        lex();
    } else if (nextToken == LEFT_PAREN) {
        lex();
        boolExpr();
        if (nextToken == RIGHT_PAREN) {
            lex();
        } else {
            error(L"Expected a right parenthesis after boolean expression.");
        }
    } else {
        error(L"Invalid boolean arithmetic factor.");
    }
    printf("Exit <boolArithFactor>\n");
}

/* Function declStmt
<declStmt> -> "tam" IDENT ["<<<" <expr>]
                | "küsurat" IDENT ["<<<" <expr>]
                | "dev" IDENT ["<<<" <expr>]
                | "hane" IDENT ["<<<" <charLit>]
                | "tümce" IDENT ["<<<" <stringLit>]
                | "mantık" IDENT ["<<<" <boolExpr>]
*/
void declStmt() {
    printf("Enter <declStmt>\n");
    if (nextToken == TYPE_INT || nextToken == TYPE_FLOAT || nextToken == TYPE_DOUBLE) {
        lex();
        if (nextToken != IDENT) {
            error(L"Expected an identifier after number type declaration.");
        } else {
            lex();
            if (nextToken == ASSIGN_OP) {
                lex();
                expr();
            } else if (nextToken != EOS) {
                error(L"Expected an assignment operator or end of line after variable declaration.");
            }
        }
    }

    else if (nextToken == TYPE_CHAR) {
        lex();
        if (nextToken != IDENT) {
            error(L"Expected an identifier after character type declaration.");
        } else {
            lex();
            if (nextToken == ASSIGN_OP) {
                lex();
                charLit();
            } else if (nextToken != EOS) {
                error(L"Expected an assignment operator or end of line after variable declaration.");
            }
        }
    }

    else if (nextToken == TYPE_STRING) {
        lex();
        if (nextToken != IDENT) {
            error(L"Expected an identifier after string type declaration.");
        } else {
            lex();
            if (nextToken == ASSIGN_OP) {
                lex();
                stringLit();
            } else if (nextToken != EOS) {
                error(L"Expected an assignment operator or end of line after variable declaration.");
            }
        }
    }

    else if (nextToken == TYPE_BOOL) {
        lex();
        if (nextToken != IDENT) {
            error(L"Expected an identifier after bool type declaration.");
        } else {
            lex();
            if (nextToken == ASSIGN_OP) {
                lex();
                boolExpr();
            } else if (nextToken != EOS) {
                error(L"Expected an assignment operator or end of line after variable declaration.");
            }
        }
    }

    else {
        error(L"Invalid type for type declaration.");
    }
    printf("Exit <declStmt>\n");
}

/* Function charLit
<charLit> -> 'CHAR'
*/
void charLit() {
    printf("Enter <charLit>\n");
    if (nextToken != APOSTROPHE) {
        error(L"Expected a single quote before character literal.");
    } else {
        lex();
        if (lexLen == 1) {
            lex();
            if (nextToken != APOSTROPHE) {
                error(L"Expected a single quote after character literal.");
            } else {
                lex();
            }
        } else {
            error(L"Character literal must be a single character.");
        }
    }
    printf("Exit <charLit>\n");
}

/* Function stringLit
<stringLit> -> "STRING"
*/
void stringLit() {
    printf("Enter <stringLit>\n");
    if (nextToken != QUOTE) {
        error(L"Expected a quote before string literal.");
    } else {
        lex();
        while (nextToken != QUOTE && nextToken != EOF) {
                lex();
        }
        if (nextToken == EOF) {
            error(L"Expected to close the string literal with a quote.");
        } else {
            /* Consume the closing quote */
            lex();
        }
    }
    printf("Exit <stringLit>\n");
}

/* Function assignStmt
<assignStmt> -> IDENT "<<<" (<expr> | <charLit> | <boolExpr>)
*/
void assignStmt() {
    printf("Enter <assignStmt>\n");
    if (nextToken != IDENT) {
        error(L"Expected an identifier for assignment.");
    } else {
        lex();
        if (nextToken != ASSIGN_OP) {
            error(L"Expected an assignment operator after identifier in assignment statement.");
        } else {
            lex();
            if (nextToken == INT_LIT || nextToken == FP_LIT || nextToken == IDENT || nextToken == LEFT_PAREN ||
                nextToken == TRUE_VAL || nextToken == FALSE_VAL || nextToken == NOT_OP) {
                /* Call boolExpr since it contains both expr and boolExpr on a non-semantic level */
                boolExpr();
            } else if (nextToken == APOSTROPHE) {
                charLit();
            } else {
                error(L"Invalid assignment value for assignment statement.");
            }
        }
    }
    printf("Exit <assignStmt>\n");
}

/* Function whileStmt
<whileStmt> -> "iken" "(" <boolExpr> ")" "{" <statementList> "}"
*/
void whileStmt() {
    printf("Enter <whileStmt>\n");
    if (nextToken != WHILE_CODE) {
        error(L"Expected \"while\" keyword.");
    } else {
        lex();
        if (nextToken != LEFT_PAREN) {
            error(L"Expected a left parenthesis after \"while\".");
        } else {
            lex();
            boolExpr();
            if (nextToken != RIGHT_PAREN) {
                error(L"Expected a right parenthesis after while condition.");
            } else {
                lex();
                if (nextToken != LEFT_CURLY) {
                    error(L"Expected a left curly brace after while loop condition.");
                } else {
                    lex();
                    statementList();
                    if (nextToken != RIGHT_CURLY) {
                        error(L"Expected a right curly brace to close \"while\" loop.");
                    } else {
                        lex();
                    }
                }
            }
        }
    }
    printf("Exit <whileStmt>\n");
}

/* Function forStmt
<forStmt> -> "sayaç" "(" <assignStmt> "." <boolExpr> "." <assignStmt> ")" "{" <statementList> "}"
*/
void forStmt() {
    printf("Enter <forStmt>\n");
    if (nextToken != FOR_CODE) {
        error(L"Expected \"for\" keyword.");
    } else {
        lex();
        if (nextToken != LEFT_PAREN) {
            error(L"Expected a left parenthesis after \"for\".");
        } else {
            lex();
            assignStmt();
            if (nextToken != EOS) {
                error(L"Expected '.' after the first assignment in for loop.");
            } else {
                lex();
                boolExpr();
                if (nextToken != EOS) {
                    error(L"Expected '.' after the boolean expression in for loop.");
                } else {
                    lex();
                    assignStmt();
                    if (nextToken != RIGHT_PAREN) {
                        error(L"Expected a right parenthesis after for loop condition.");
                    } else {
                        lex();
                        if (nextToken != LEFT_CURLY) {
                            error(L"Expected a left curly brace after for loop condition.");
                        } else {
                            lex();
                            statementList();
                            if (nextToken != RIGHT_CURLY) {
                                error(L"Expected a right curly brace to close \"for\" loop.");
                            } else {
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