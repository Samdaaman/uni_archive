all: threading.pdf

%.pdf: %.tex
	pdflatex -shell-escape $*
	pdflatex -shell-escape $*

clean:
	rm *.pdf *.aux *.log *.out

