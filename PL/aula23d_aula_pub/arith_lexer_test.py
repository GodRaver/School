# arith_lexer_test.py
from arith_lexer import ArithLexer

al = ArithLexer()
al.build()
al.input("F=10; G=3*-5+7;") # E:=3*5+7;F:=10")	 #2++3") #"(3+5)*7")

while True:
    tk = al.token() 
    if not tk: 
        break
    print(tk,end=" ")

    