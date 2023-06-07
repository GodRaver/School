import ply.lex as lex
import ply.yacc as yacc

reserved = {
    'VAR': 'VAR',
    'PRINT': 'PRINT'
}

# Tokens
tokens = [
    'ID',
    'NUM',
    'STRING',
    'ASSIGN',
    'ADD',
    'SUB',
    'MUL',
    'DIV',
    'LPAREN',
    'RPAREN',
    'SEMICOLON',
    'COMMA',
] + list(reserved.values())

# Ignored characters
t_ignore = ' \t'

# Token definitions
t_ASSIGN = r'='
t_ADD = r'\+'
t_SUB = r'-'
t_MUL = r'\*'
t_DIV = r'/'
t_LPAREN = r'\('
t_RPAREN = r'\)'
t_SEMICOLON = r';'
t_COMMA = r','

def t_ID(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    t.type = reserved.get(t.value, 'ID')
    return t

def t_NUM(t):
    r'\d+'
    t.value = int(t.value)
    return t

def t_STRING(t):
    r'"[^"]*"'
    t.value = t.value[1:-1]
    return t

def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

def t_error(t):
    print(f"Illegal character '{t.value[0]}'")
    t.lexer.skip(1)

# Grammar rules
def p_program(p):
    'program : statement_list'
    p[0] = p[1]

def p_statement_list(p):
    '''statement_list : statement
                      | statement_list statement'''
    if len(p) == 2:
        p[0] = [p[1]]
    else:
        p[0] = p[1] + [p[2]]

def p_statement(p):
    '''statement : variable_declaration
                 | assignment
                 | write_instruction'''
    p[0] = p[1]

def p_variable_declaration(p):
    'variable_declaration : VAR ID ASSIGN expression SEMICOLON'
    p[0] = ('VAR_DECL', p[2], p[4])

def p_assignment(p):
    'assignment : ID ASSIGN expression SEMICOLON'
    p[0] = ('ASSIGN', p[1], p[3])

def p_write_instruction(p):
    'write_instruction : PRINT expression_list SEMICOLON'
    p[0] = ('WRITE', p[2])

def p_expression_list(p):
    '''expression_list : expression
                       | expression_list COMMA expression'''
    if len(p) == 2:
        p[0] = [p[1]]
    else:
        p[0] = p[1] + [p[3]]

def p_expression(p):
    '''expression : ID
                  | NUM
                  | STRING
                  | LPAREN expression RPAREN
                  | expression ADD expression
                  | expression SUB expression
                  | expression MUL expression
                  | expression DIV expression'''
    if len(p) == 2:
        p[0] = p[1]
    elif p[1] == '(':
        p[0] = p[2]
    else:
        p[0] = (p[2], p[1], p[3])

def p_error(p):
    print("Syntax error")

# Build the lexer and parser
lexer = lex.lex()
parser = yacc.yacc()

# Example input
data = '''
VAR x = 10;
VAR y = 5;
PRINT "The sum is:", x + y, "ola";
VAR z = 6;
PRINT z + x + y;
VAR u = z;
PRINT u;
'''

# Parse the input
result = parser.parse(data, lexer=lexer)

def evaluate_expression(expr):
    if isinstance(expr, tuple):
        operator = expr[0]
        operand1 = evaluate_expression(expr[1])
        operand2 = evaluate_expression(expr[2])
        if operator == '+':
            return operand1 + operand2
        elif operator == '-':
            return operand1 - operand2
        elif operator == '*':
            return operand1 * operand2
        elif operator == '/':
            return operand1 / operand2
    elif isinstance(expr, str):
        if expr in result_dict:
            return result_dict[expr]
        else:
            return expr

result_dict = {}
for statement in result:
    if statement[0] == 'VAR_DECL':
        result_dict[statement[1]] = evaluate_expression(statement[2])
    elif statement[0] == 'ASSIGN':
        result_dict[statement[1]] = evaluate_expression(statement[2])
    elif statement[0] == 'WRITE':
        output = []
        for expr in statement[1]:
            value = evaluate_expression(expr)
            if isinstance(value, str):
                output.append(value)
            else:
                output.append(str(value))
        print(' '.join(output))
