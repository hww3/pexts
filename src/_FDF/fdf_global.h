#ifndef __FDF_GLOBAL_H
#define __FDF_GLOBAL_H

#if PIKE_MAJOR_VERSION == 7 && PIKE_MINOR_VERSION > 0
#define WRONG_NUM_OF_ARGS(name, nargs, nexpec) wrong_number_of_args_error(name, nargs, nexpec)
#else
#define WRONG_NUM_OF_ARGS(name, nargs, nexpec) \
    Pike_error("%s: wrong number of arguments (%d found, %d expected)\n", \
               name, nargs, nexpec)
#endif

#define FDF_MAGIC 0x6D464446
#endif
