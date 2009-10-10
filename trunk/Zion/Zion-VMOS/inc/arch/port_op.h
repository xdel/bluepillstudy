// ((void (*)(u16, u8 ))out_byte)(port, value);
extern char out_byte[];

// ((u8 (*)(u16))in_byte)(port);
extern char in_byte[];

// ((void (*)(u16, void*, int))port_read)(port, buf, n);
extern char port_read[];

// ((void (*)(u16, void*, int))port_write)(port, buf, n);
extern char port_write[];
