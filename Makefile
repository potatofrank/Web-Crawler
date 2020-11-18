all: clean
	gcc main.c crawler.c -o crawler

clean:
	rm -f crawler