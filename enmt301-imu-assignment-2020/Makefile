all: report-template.pdf

%.pdf: %.tex sinewave.pgf
	pdflatex $<

%.rtf: %.tex
	-latex2rtf -o $@ $<

%.pgf: %.py
	python $^



