AC_INIT(pdf.c)
AC_CONFIG_HEADER(pdf_config.h)

CAUDIUM_MODULE_INIT()

AH_BOTTOM([
#if defined(HAVE_PDFLIB_H) && defined(HAVE_LIBPDF)
#define HAVE_PDFLIB
#include <pdflib.h>

#define THIS ((PDF_storage *)Pike_fp->current_storage)

typedef struct
{
  PDF *pdf;
  char *storage;
  char *tail;
  size_t size;
  char *filename;
} PDF_storage;

#ifndef ADD_STORAGE
/* Pike 0.6 */
#define ADD_STORAGE(x) add_storage(sizeof(x))
#endif
size_t writeproc( PDF *p, void *data, size_t size );
void f_pdf_create(INT32 args);
void f_set_info(INT32 args);
void f_begin_page(INT32 args);
void f_end_page(INT32 args);
void f_close(INT32 args);
void f_findfont(INT32 args);
void f_setfont(INT32 args);
void f_show(INT32 args);
void f_show_boxed(INT32 args);
void f_continue_text(INT32 args);
void f_set_text_pos(INT32 args);
void f_generate(INT32 args);
void f_stringwidth(INT32);

/* Graphics Functions */
void f_setdash(INT32);
void f_setlinewidth(INT32);
void f_moveto(INT32);
void f_lineto(INT32);
void f_curveto(INT32);
void f_circle(INT32);
void f_arc(INT32);
void f_rect(INT32);
void f_stroke(INT32);
void f_fill(INT32);

void f_setgray_fill(INT32);
void f_setgray_stroke(INT32);
void f_setgray(INT32);

void f_setrgbcolor_fill(INT32);
void f_setrgbcolor_stroke(INT32);
void f_setrgbcolor(INT32);

void f_get_value(INT32);
void f_set_value(INT32);

void f_get_parameter(INT32);
void f_set_parameter(INT32);

void f_add_bookmark(INT32);

#endif /* HAVE_PDFLIB */

/* pike module functions */
void pike_module_init(void);
void pike_module_exit(void);
])

AC_CHECK_HEADERS( pdflib.h )
AC_CHECK_LIB(pdf, PDF_new)


AC_OUTPUT(Makefile.pre, [${MAKE:-make} -f Makefile.pre depend])


dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
