all: compile

compile: leak.c
	gcc leak.c -Os -Wall -o leak -mrtm

clean:
	@rm -rf leak