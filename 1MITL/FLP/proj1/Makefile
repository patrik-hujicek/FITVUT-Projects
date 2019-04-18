OUT=simplify-bkg

all:
	ghc --make $(OUT).hs -o $(OUT)
clean:
	rm -f $(OUT) $(OUT).hi $(OUT).o flp-fun-xbolva00.zip
pack:
	zip flp-fun-xbolva00.zip Makefile $(OUT).hs README -r