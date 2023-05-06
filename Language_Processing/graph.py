import graphviz

  # create a new Graphviz graph
g = graphviz.Digraph(format='png')
g.attr(rankdir='LR')
  # add nodes to the graph
g.node('q0')
g.node('q1')
g.node('q2', shape='doublecircle')

  # add edges to the graph
g.edge('q0', 'q1', label='0')
g.edge('q1', 'q2', label='1')
g.edge('q2', 'q0', label='0')
g.edge('q2', 'q1', label='1')

# render the graph to a PNG image file
g.render(filename='automaton', directory='.', cleanup=True)

