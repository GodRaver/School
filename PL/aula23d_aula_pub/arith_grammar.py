# arith_grammar.py
from arith_lexer import ArithLexer
import ply.yacc as pyacc


class ArithGrammar:
    precedence = (
       # ( 'nonassoc', '>' ),
        ( 'left' , '+', '-'),
         ("left", '*'),
         ("right", 'simetrico' ),
         )
    
    def __init__(self):
        self.yacc = None
        self.lexer = None
        self.tokens = None

    def build(self, **kwargs):
        self.lexer = ArithLexer()
        self.lexer.build(**kwargs)
        self.tokens = self.lexer.tokens
        self.yacc = pyacc.yacc(module=self, **kwargs)

    def parse(self, string):
        self.lexer.input(string)
        return self.yacc.parse(lexer=self.lexer.lexer)

    def p_s(self, p):
        """ S : LstV ';' """
        p[0] = p[1]
             
    def p_expr_tail(self, p):
        """ LstV :  LstV ';' Inst """
        lstArgs =  p[1]['args']
        lstArgs.append(p[3])
        p[0] = dict(op='seq', args= lstArgs  )
        
    def p_expr_head(self, p):  
        """ LstV :  Inst """
        p[0] = dict(op='seq', args= [ p[1] ]  )

    def p_expr_inst_esc_string(self, p):
        """Inst :  escrever string"""
        p[0] = {'op': 'esc', 'args': [p[2]]}
    
    def p_expr_inst_atr(self,p):    
        """ Inst : V  """ 
        p[0] = p[1]
        
    def p_expr_inst_esc(self,p):    
        """ Inst :  escrever E  """    
        p[0] = {'op': 'esc', 'args': [ p[2] ] }
         
    def p_expr_atrib(self,p):
        """ V : varid '=' E """
        p[0]= dict(op='atr',args= [ p[1] , p[3]] )
         
    def p_expr_op(self, p):
        """ E : E '+' E  
              | E '-' E 
              | E '*' E """       
        p[0]=dict(op=p[2],args= [ p[1] , p[3]] )

        
    def p_expr_sinalmenos(self, p): 
        " E : '-' E   %prec simetrico "
        # p[0] = -p[2]
        p[0] = dict(op='-', args=[p[2]]) 
        
    def p_expr_pare(self, p):      
        """ E : '(' E ')' """
        p[0]= p[2]
    
    def p_expr_num(self, p):      
        """ E :  num  """
        p[0]= p[1] 
    
    def p_expr_var(self, p):      
        """ E :  varid  """  
        p[0]= {'var': p[1] }     
    
    def p_error(self, p):
        if p:
            print(f"Syntax error: unexpected '{p.type}'")
        else:
            print("Syntax error: unexpected end of file")
        exit(1)
