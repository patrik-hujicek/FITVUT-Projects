OUT=flp19-log

all:
	swipl -q -g main -o $(OUT) -c $(OUT).pl
run: all
	./$(OUT) < 6tahov.txt
clean:
	rm -f $(OUT) flp-log-xbolva00.zip
pack:
	zip flp-log-xbolva00.zip Makefile $(OUT).pl *.txt README -r 
