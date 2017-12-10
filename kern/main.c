void PrintString() {
	char *p = (char *)0xb8000;
	const char *str = "Hello world!";
	while (*str) {
		*p++ = *str++;
		*p++ = 0x07;
	}
}
