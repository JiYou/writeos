#define VIDEO_RAM_START 0xb8000

void ClearScreen(void) {
	unsigned char *video = (unsigned char *)(VIDEO_RAM_START);
	const int MaxColumns = 80;
	const int MaxRows = 25;
	volatile int i = 0;
	for (i = 0; i < MaxColumns * MaxRows; i++) {
		*video++ = 0x20;
		*video++ = 0x07;
	}
}

inline void putChar(char **addr, char c) {
	**addr = c;
	++(*addr);
	**addr = 0x07;
	++(*addr);
}

void PrintString(void) {
        char *video= (char *)(VIDEO_RAM_START);
	/*
 	 * 不要在这里使用char *str = "Hello"
 	 * 来进行输出，这是因为boot sector只拷贝
 	 * 代码段，数据段是不会拷贝的。
 	 */
	putChar(&video, 'H');
	putChar(&video, 'e');
	putChar(&video, 'l');
	putChar(&video, 'l');
	putChar(&video, 'o');
}
