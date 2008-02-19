/*
 * Use latin letters for GOST output.
 */
extern int gost_latin;

extern const unsigned char itm_to_gost[], gost_to_itm[];
extern const unsigned char text_to_gost[];

void gost_putc (unsigned char, FILE*);
void gost_write (unsigned char*, int, FILE*);
unsigned char unicode_to_gost (unsigned short);
unsigned char utf8_to_gost (unsigned char**);
void utf8_puts (char*, FILE*);
int unicode_getc (FILE*);
void set_input_encoding (char*);
