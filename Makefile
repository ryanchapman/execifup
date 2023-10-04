CC=gcc

execifup:
	$(CC) -o execifup execifup.c

.PHONY: clean

clean:
	rm -f execifup core
