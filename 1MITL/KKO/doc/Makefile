OUT=dokumentacia

all:
	vlna $(OUT).tex
	pdflatex $(OUT).tex
	pdflatex $(OUT).tex
	cp $(OUT).pdf ../
clean:
	rm --force --verbose *.aux *.synctex.gz *.log *.out *.te~ *.toc
