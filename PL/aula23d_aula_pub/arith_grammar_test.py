# arith.py
from arith_grammar import ArithGrammar
from pprint import PrettyPrinter

pp = PrettyPrinter(sort_dicts=False)

ag = ArithGrammar()
ag.build()

exemplos = [ # exemplos a avaliar de forma independente... 
            "E=10;",
            "F=10; G=3*-5+7;",   #"3*-5+7"  3*(-5)+7
            "G=3*(5+7); H=G*2;",
            "I=-2; J=I*-2; K=I+1;" 
            ]
for frase in exemplos:
    print(f"----------------------")
    print(f"--- frase '{frase}'")
    res = ag.parse ( frase ) 
    print("resultado: ")
    pp.pprint(res)
