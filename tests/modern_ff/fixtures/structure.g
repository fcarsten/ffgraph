{forward targets, parallel edges, loop, duplicate declaration and undeclared target}
GRAPH "structure" = UNDIRECTED
1 {$ SN FF_3D -10 20 30 FF_R 7 FF_C "red" $} "alpha"
 2 {$ FF_WE -125 $} "first"
 2 {$ FF_WE 250 $} "parallel"
 1 "loop"
 99 "undeclared";
2 {$ EN NP 40 -50 $} "beta";
2 {$ EN $} "renamed beta";
-7 "isolated";
END
