import ply.lex as lex
import ply.yacc as yacc
import pydot

reserved = {
    'VAR': 'VAR',
    'PRINT': 'PRINT',
    'FUNCTION': 'FUNCTION',
    'CALL': 'CALL'
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
    'LBRACE',
    'RBRACE',
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
t_LBRACE = r'{'
t_RBRACE = r'}'
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
                 | write_instruction
                 | function_definition
                 | function_call'''
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

def p_function_definition(p):
    'function_definition : FUNCTION ID LPAREN parameter_list RPAREN LBRACE statement_list RBRACE'
    p[0] = ('FUNCTION_DEF', p[2], p[4], p[7])

def p_parameter_list(p):
    '''parameter_list : ID
                      | parameter_list COMMA ID'''
    if len(p) == 2:
        p[0] = [p[1]]
    else:
        p[0] = p[1] + [p[3]]

def p_function_call(p):
    'function_call : CALL ID LPAREN argument_list RPAREN SEMICOLON'
    p[0] = ('FUNCTION_CALL', p[2], p[4])

def p_argument_list(p):
    '''argument_list : expression
                     | argument_list COMMA expression'''
    if len(p) == 2:
        p[0] = [p[1]]
    else:
        p[0] = p[1] + [p[3]]

def p_error(p):
    print("Syntax error")

# Build the lexer and parser
lexer = lex.lex()
parser = yacc.yacc()

# Read from file
filename = 'exemplo.ea'
with open(filename, 'r') as file:
    data = file.read()

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
            return expr  # Return the string value as is
    return expr

result_dict = {}
def execute_statement(statement):
    if statement[0] == 'VAR_DECL':
        result_dict[statement[1]] = evaluate_expression(statement[2])
    elif statement[0] == 'ASSIGN':
        result_dict[statement[1]] = evaluate_expression(statement[2])
    elif statement[0] == 'WRITE':
        output = []
        for expr in statement[1]:
            output.append(str(evaluate_expression(expr)))
        print(' '.join(output))
    elif statement[0] == 'FUNCTION_DEF':
        func_name = statement[1]
        params = statement[2]
        func_body = statement[3]
        result_dict[func_name] = (params, func_body)
    elif statement[0] == 'FUNCTION_CALL':
        func_name = statement[1]
        arguments = statement[2]
        if func_name in result_dict:
            function_def = result_dict[func_name]
            params = function_def[0]
            func_body = function_def[1]
            if len(params) == len(arguments):
                func_dict = {}
                for i in range(len(params)):
                    func_dict[params[i]] = evaluate_expression(arguments[i])
                result_dict.update(func_dict)
                for stmt in func_body:
                    execute_statement(stmt)
                for param in params:
                    result_dict.pop(param)  # Remove function call context variables
            else:
                print(f"Error: Function '{func_name}' requires {len(params)} arguments, but {len(arguments)} were provided.")
        else:
            print(f"Error: Function '{func_name}' is not defined.")

# Execute each statement
for statement in result:
    execute_statement(statement)


graph = pydot.Dot(graph_type='graph')

def create_syntax_tree(node, parent=None):
    if isinstance(node, tuple):
        node_label = node[0]  # Extract the label from the tuple
        node_id = str(id(node))
        graph_node = pydot.Node(node_id, label=str(node_label))
        graph.add_node(graph_node)
        if parent is not None:
            graph.add_edge(pydot.Edge(parent, graph_node))
        for child in node[1:]:
            create_syntax_tree(child, parent=graph_node)
    elif isinstance(node, list):
        for item in node:
            create_syntax_tree(item, parent=parent)
    elif isinstance(node, str):
        node_id = str(id(node))
        graph_node = pydot.Node(node_id, label=str(node))
        graph.add_node(graph_node)
        if parent is not None:
            graph.add_edge(pydot.Edge(parent, graph_node))
    else:
        return  # Skip numerical values


# Create the syntax tree
create_syntax_tree(result)

# Save the syntax tree as a PNG file
graph.write_png('syntax_tree.png')
print("Syntax tree saved as syntax_tree.png")
   