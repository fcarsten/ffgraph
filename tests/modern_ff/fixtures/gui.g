GRAPH "FF GUI integration" = DIRECTED
{$ FF_LO 12 -8 FF_SC 125 FF_RX 50 FF_RY -75 FF_RZ 100 $}
1 {$ NP 20 40 SN FF_R 8 FF_C "blue" $} "source"
 2 {$ FF_3D 20 40 0 55 65 5 100 80 0 FF_WE 175 FF_C "green" $} "bend"
 2 {$ FF_WE -50 $} "parallel";
2 {$ FF_3D 100 80 10 EN $} "target" 2 "loop";
3 {$ NP 160 120 $} "isolated";
END
