# Syntax-valid GML that must fail semantic preflight without mutating a graph.
graph [
  directed 1
  node [ id 1 label "existing" ]
  edge [ source 1 target 999 label "unresolved target" ]
]
