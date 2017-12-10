void PrintString() {
	char *p = (char *)0xb8000;
	*p = 'H';
	++p;
	*p = 0x07;
}
