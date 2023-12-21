all:
	gcc -O3 -g -Wall -Wextra visualize.c -o visualize -lfann -lm
#~	gcc -g -Wall -Wextra visualize.c -o visualize -lfann -lm

clean:
	rm visualize
