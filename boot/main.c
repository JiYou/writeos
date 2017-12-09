void ClearScreen() {
	unsigned char *start = (unsigned char *)(0xb8000);
	const int MaxColumns = 80;
	const int MaxRows = 25;
	int i = 0;
	for (i = 0; i < MaxColumns * MaxRows; i++) {
		*start = 0x20;
		start++;
		*start = 0x07;
		start++;
	}
}
