import ply.lex as lex
import ply.yacc as yacc

# Variable storage
variables = {}
declared_variables = set()

# Reserved words
reserved = {
    'DECLARE': 'DECLARE',
    'PRINT': 'PRINT'
}

# Tokens list
tokens = [
    'ID',
    'NUM',
    'STRING',
    'ADD',
    'SUB',
    'MUL',
    'DIV',
    'ASSIGN',
    'SEMICOLON',
    'NEWLINE',
    'COMMA',
    'LPAREN',
    'RPAREN'
] + list(reserved.values())

# Lexer rules

# Arithmetic operators
t_ADD = r'\+'
t_SUB = r'-'
t_MUL = r'\*'
t_DIV = r'/'

# Assignment operator
t_ASSIGN = r'='

# Semicolon and newline
t_SEMICOLON = r';'
t_NEWLINE = r'\n'

# Comma and parentheses
t_COMMA = r','
t_LPAREN = r'\('
t_RPAREN = r'\)'

# Ignored characters (spaces and tabs)
t_ignore = ' \t'

# Identifiers and reserved words
def t_ID(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    t.type = reserved.get(t.value, 'ID')
    return t

# Numbers
def t_NUM(t):
    r'\d+'
    t.value = int(t.value)
    return t

# Strings
def t_STRING(t):
    r'\".*?\"'
    t.value = t.value[1:-1]
    return t

# Error handling
def t_error(t):
    print(f"Invalid character: {t.value[0]}")
    t.lexer.skip(1)

# Parser rules

# Program rule
def p_program(p):
    'program : instruction_list'
    p[0] = p[1]

# Instruction list rules
def p_instruction_list(p):
    '''instruction_list : instruction
                        | instruction_list instruction'''
    if len(p) == 2:
        p[0] = [p[1]]
    else:
        p[0] = p[1] + [p[2]]

# Instruction rules
def p_instruction(p):
    '''instruction : declaration
                   | assignment
                   | print_instruction'''
    p[0] = p[1]

# Declaration rules
def p_declaration(p):
    'declaration : DECLARE ID_LIST SEMICOLON'
    for var_name in p[2]:
        if var_name not in declared_variables:
            declared_variables.add(var_name)
            variables[var_name] = None
        else:
            print(f"Error: Variable '{var_name}' already declared.")

def p_id_list(p):
    '''ID_LIST : ID
               | ID_LIST COMMA ID'''
    if len(p) == 2:
        p[0] = [p[1]]
    else:
        p[0] = p[1] + [p[3]]

# Assignment rules
def p_assignment(p):
    'assignment : ID ASSIGN expression SEMICOLON'
    var_name = p[1]
    expression_value = p[3]
    if var_name in declared_variables:
        variables[var_name] = expression_value
    else:
        print(f"Error: Variable '{var_name}' not declared.")

# Expression rules
def p_expression(p):
    '''expression : expression ADD term
                  | expression SUB term
                  | term'''
    if len(p) == 4:
        if p[2] == '+':
            p[0] = p[1] + p[3]
        elif p[2] == '-':
            p[0] = p[1] - p[3]
    else:
        p[0] = p[1]

def p_term(p):
    '''term : term MUL factor
            | term DIV factor
            | factor'''
    if len(p) == 4:
        if p[2] == '*':
            p[0] = p[1] * p[3]
        elif p[2] == '/':
            p[0] = p[1] / p[3]
    else:
        p[0] = p[1]

def p_factor(p):
    '''factor : LPAREN expression RPAREN
              | value'''
    if len(p) == 4:
        p[0] = p[2]
    else:
        p[0] = p[1]

def p_value(p):
    '''value : NUM
             | STRING
             | ID'''
    if isinstance(p[1], str):
        var_name = p[1]
        if var_name in declared_variables:
            p[0] = variables[var_name]
        else:
            print(f"Error: Variable '{var_name}' not declared.")
    else:
        p[0] = p[1]

# Print instruction rule
def p_print_instruction(p):
    'print_instruction : PRINT expression SEMICOLON'
    print(p[2])

# Error handling
def p_error(p):
    if p:
        print(f"Syntax error at line {p.lineno}: Unexpected token '{p.value}'")
    else:
        print("Syntax error: Unexpected end of input")

# Build the lexer and parser
lexer = lex.lex()
parser = yacc.yacc()

# Sample program
program_code = '''
DECLARE x, y;
x = 5;
y = 10;
PRINT "The sum is:", x + y, "ola";
PRINT "The sum is:", "ola";
'''

# Tokenize the program code
lexer.input(program_code)

# Parse the program
program = parser.parse(program_code)

# Execute the program
program.execute()
