from lib2to3.pgen2 import literals
import ply.lex as plex


class ArithLexer:
    tokens=("VAR", "NUM", "ESCREVER",)
    literals=['+','-','*','/',]
