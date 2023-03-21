V = set([range(0,9)])
Q = {"q0", "q1"}
#tt = {"q0": {"a":"q1","b":"q3"},
#      "q1": {"a":"q3","b":"q2"},
#      "q2": {"a":"q2","b":"q2"},
#      "q3": {"a":"q3","b":"q3"} }
tt = {"q0": {"digito":"q1"},
      "q1": {"digito":"q1"}}
q0 = "q0"
F = {"q1"}




def reconhece(palavra):
    alpha = q0  
    while len(palavra)>0  and alpha !="ERRO" :
        simbolo_atual = palavra[0]
        alpha = tt[alpha][simbolo_atual] if simbolo_atual in V else "ERRO"
        palavra = palavra[1:]
    return (len(palavra)== 0) and (alpha in F)


print(f' {reconhece("aba")}')
print(f' {reconhece("ba")}')
print(f' {reconhece("abac")}')
