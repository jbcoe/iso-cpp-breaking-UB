all: D1093R1.pdf

D1093R1.pdf: readme.md
	pandoc readme.md --pdf-engine=xelatex -o D1093R1.pdf

clean:
	rm -f D1093R1.pdf
