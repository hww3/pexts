#ifndef AT_CONFIG_H
#define AT_CONFIG_H

@TOP@

@BOTTOM@

#ifndef ADD_STORAGE
#define ADD_STORAGE(x) add_storage(sizeof(x))
#endif

/* Define if your system has known quotactl support */
#undef HAVE_BZLIB_H

#endif

void pike_module_init(void);
void pike_module_exit(void);

