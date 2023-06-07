import ply.lex as lex
import ply.yacc as yacc
from analise_lexica import tokens
from analise_sintatica import parser


def run_program(input_file):
    with open(input_file, 'r') as file:
        data = file.read()

    lexer.input(data)
    program = parser.parse(data, lexer=lexer)
    return program

# Build the lexer
lexer = lex.lex()

# Build the parser
parser = yacc.yacc()

# Execution of the program
input_file = 'entrada.ea'
program = run_program(input_file)
program.execute()
