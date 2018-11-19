all:
	gcc main.c -o main
	gcc inputGenerator.c -o inputGenerator

scaleSet.out:
	./main < scaleSet.config

constant.out:
	./main < constant.config

plot-graph: scaleSet.out constant.out
	gnuplot -e "set terminal png size 960,600; set output 'Graph.png'; plot 'scaleSet.out', 'constant.out'"

clear:
	rm *.out inputGenerator
