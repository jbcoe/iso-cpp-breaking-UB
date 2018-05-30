all: P1093.pdf

P1093.pdf: readme.md
	pandoc readme.md --pdf-engine=xelatex -o P1093.pdf

clean:
	rm -f P1093.pdf
