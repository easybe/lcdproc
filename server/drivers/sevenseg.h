#ifndef SEVENSEG_H
#define SEVENSEG_H

MODULE_EXPORT int sevenseg_init(Driver *drvthis);
MODULE_EXPORT void sevenseg_close(Driver *drvthis);
MODULE_EXPORT int sevenseg_width(Driver *drvthis);
MODULE_EXPORT int sevenseg_height(Driver *drvthis);
MODULE_EXPORT void sevenseg_clear(Driver *drvthis);
MODULE_EXPORT void sevenseg_flush(Driver *drvthis);
MODULE_EXPORT void sevenseg_string(Driver *drvthis, int x, int y, const char string[]);
MODULE_EXPORT void sevenseg_chr(Driver *drvthis, int x, int y, char c);
MODULE_EXPORT void sevenseg_set_contrast(Driver *drvthis, int promille);
MODULE_EXPORT void sevenseg_backlight(Driver *drvthis, int on);
MODULE_EXPORT const char *sevenseg_get_info(Driver *drvthis);

#define SEVENSEG_DEFAULT_SIZE "20x4"

#endif
