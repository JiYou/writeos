all: temp.bin

temp.bin: hello.asm
	nasm hello.asm -o temp.bin

.PHONY: clean
clean:
	rm -rf temp.bin

.PHONY: run
run:
	bochs
