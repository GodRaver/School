# arith_eval

class ArithEval:

    # guardar as atribuições de variáveis
    symbols = {}

    operators = {
        "+": lambda args: args[0] + args[1],
        "-": lambda args: args[0] - args[1],
        "*": lambda args: args[0] * args[1],
        "seq": lambda args: args[-1], # 
        "atr": lambda args: ArithEval._attrib(args),
        "esc": lambda args: print(args[0]),
    }

    @staticmethod
    def _attrib(args):
        varid = args[0]
        value = args[1] 
        ArithEval.symbols[varid] = value  
        return None
    
    @staticmethod
    def evaluate(ast):
        if type(ast) is int: 
            return ast
        if type(ast) is dict:
            return ArithEval._eval_operator(ast)
        if type(ast) is str: 
            return ast
        raise Exception(f"Unknown AST type")
        
    @staticmethod
    def _eval_operator(ast):
        if 'op' in ast:
            op = ast["op"]
            args = [ArithEval.evaluate(a) for a in ast['args']]
            if op in ArithEval.operators:
                func = ArithEval.operators[op]
                return func(args)
            else:
                raise Exception(f"Unknown operator {op}")
            
        if 'var' in ast:
            varid = ast["var"]     #ast={ 'var': "A" } =>   ast["var"]   varid="A"  
            if varid in ArithEval.symbols:  # "A" in symbols { 'A':10 } 
                return ArithEval.symbols[varid]   # 10   
            raise Exception(f"error: '{varid}' undeclared (first use in this function)")    
        raise Exception('Undefined AST')

