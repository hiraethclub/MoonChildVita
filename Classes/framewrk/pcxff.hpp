#ifndef _PCXFF_HPP
#define _PCXFF_HPP

VG_BOOLEAN PCX_ff_stat(const char *fname, UINT16 *width, UINT16 *height);
VG_BOOLEAN PCX_ff_load(const char *fname, BYTE *buffer, BYTE *palette);
void       PCX_ff_dump(const char *filename, FILE *out);

#endif /* _PCXFF_HPP */
