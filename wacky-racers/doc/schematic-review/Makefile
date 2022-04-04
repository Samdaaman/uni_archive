
all: schematic-review.pdf

%.pdf: %.tex
	mkdir -p build
	pdflatex -halt-on-error $<
	pdflatex -halt-on-error $<
