/* Glue for the libmcrypt encryption library.
   Written by David Hedbor <david@hedbor.org>
*/
void pike_module_init(void);
void pike_module_exit(void);


#include "global.h"
RCSID("$Id$");
#include "caudium_util.h"

#ifdef HAVE_LIBMCRYPT
#include <stdlib.h>
#include <mcrypt.h>

#ifndef ARG
/* Get argument # _n_ */
#define ARG(_n_) sp[-((args - _n_) + 1)]
#endif

#define MODE_NONE    0
#define MODE_DECRYPT 1
#define MODE_ENCRYPT 2

typedef struct
{
  MCRYPT td;
  char* IV, *block_buffer;
  struct pike_string *algo;
  struct pike_string *mode;
  struct pike_string *key;
  INT8 inited, runmode;
}  mcrypt_data;

static struct program *mcrypt_program;

#define THIS ((mcrypt_data*) (Pike_fp->current_storage))

static void f_create(INT32 args)
{
  if(THIS->algo != NULL) {
    Pike_error("Create already called!\n");
  }
  switch(args) {
  case 2:
    if(ARG(2).type != T_STRING) {
      Pike_error("Invalid argument 2, expected string.\n");
    }
    add_ref(THIS->mode = ARG(2).u.string);
    
    /* FALL THROUGH */
  case 1:
    if(THIS->mode == NULL) {
      THIS->mode = make_shared_binary_string("ecb", 3);
    }
    if(ARG(1).type != T_STRING) {
      Pike_error("Invalid argument 1, expected string.\n");
    }
    add_ref(THIS->algo = ARG(1).u.string);
    break;
  default:
    Pike_error("Invalid number of arguments to create()\n");
  }

  THIS->td = mcrypt_module_open(THIS->algo->str, /* Algorithm */
				NULL, /* Algorithm dir */
				THIS->mode->str, /* mode */
				NULL);
  if(THIS->td == MCRYPT_FAILED) {
    Pike_error("Failed to initialize algorithm.\n");
  }
  pop_n_elems(args-1); /* Leave one on stack */
} /* f_create */

static void f_set_key(INT32 args) {
  int err=0;
  int ivsize=0;
  if(args < 1) {
    Pike_error("Too few arguments to se_key().\n");
  }
  if(ARG(1).type != T_STRING) {
    Pike_error("Invalid argument 1 to set_key(), expected string.\n");
  }
  if(THIS->inited) {
    mcrypt_generic_deinit(THIS->td);
    THIS->inited = 0;
    THIS->runmode = MODE_NONE;
  }
  if(THIS->key) {
    free_string(THIS->key);
    THIS->key = NULL;
  }
  ivsize = mcrypt_enc_get_iv_size(THIS->td);
  THIS->IV = realloc(THIS->IV, ivsize);
  if(!THIS->IV) {
    Pike_error("Out of memory.\n");
  }
  
  if(args > 1) {
    if(ARG(2).type != T_STRING) {
      Pike_error("Invalid argument 2 to set_key(), expected string.\n");
    }
    if((ARG(2).u.string->len <<  ARG(2).u.string->size_shift) < ivsize)
      Pike_error("Random data string in argument 2 is too short. "
		 "Need %d bytes.\n", ivsize);
    memcpy(THIS->IV, ARG(2).u.string->str, ivsize);
  } else {
    int i;
    /* Populate with pseudo-random data */
    for(i = 0; i < ivsize; i++) {
      THIS->IV[i] = my_rand()%256;
    }
  }

  if(!ivsize && THIS->IV != NULL) {
    free(THIS->IV);
    THIS->IV = NULL;
  }
  add_ref(THIS->key = ARG(1).u.string);
  pop_n_elems(args);
  push_int(0);
}

static INLINE void generic_init(int deinit) {
  int err;
  if(deinit && THIS->inited) {
    mcrypt_generic_deinit(THIS->td);
    THIS->inited = 0;
    THIS->runmode = MODE_NONE;
  }
    
  if(THIS->key != NULL) {
    if(!THIS->inited) {
      if(!mcrypt_enc_mode_has_iv(THIS->td)) {
	MEMSET(THIS->IV, 0, mcrypt_enc_get_iv_size(THIS->td));
      }
      err = mcrypt_generic_init(THIS->td, THIS->key->str,
				THIS->key->len <<
				THIS->key->size_shift, 
				THIS->IV);
      if(err) {
	Pike_error("Error while initializing crypt: %s\n", mcrypt_strerror(err));
      }    
      THIS->inited = 1;
    }
  } else  {
    Pike_error("You need to set the key using set_key() before encrypting!\n");
  }
}

static void f_encrypt(INT32 args) {
  int blocksize, needeblocks, padneeded, len;
  char *block_buffer, *IV = NULL;
  int err;
  int ivsize = 0;
  MCRYPT td = THIS->td;

  if(!THIS->inited) {
    generic_init(0);
  }
  if(THIS->runmode == MODE_DECRYPT) {
    Pike_error("This object is in decryption mode!\n");
  } else if(!args || ARG(1).type != T_STRING) {
    Pike_error("Invalid or missing argument to encrypt(), expected string.\n");
  }
  len = ARG(1).u.string->len << ARG(1).u.string->size_shift;
  blocksize =  mcrypt_enc_get_block_size(THIS->td);
  if(mcrypt_enc_is_block_mode(td)) {
    if( (len % blocksize) != 0 ) {
      Pike_error("Invalid length for indata. Should be a multiple of %d\n",
		 blocksize);
    }
  }
  if(THIS->runmode == MODE_NONE) {
    if(mcrypt_enc_mode_has_iv(td)) {
      ivsize = mcrypt_enc_get_iv_size(THIS->td);
    }
    THIS->runmode = MODE_ENCRYPT;
  }
  THIS->block_buffer = malloc(len);
  if(!THIS->block_buffer) {
    Pike_error("Failed to allocate block buffer!\n");
  }
  memcpy(THIS->block_buffer, ARG(1).u.string->str, len);
  block_buffer = THIS->block_buffer;
  THREADS_ALLOW();
  err = mcrypt_generic (td, block_buffer, len);
  THREADS_DISALLOW();
  if(err) {
    Pike_error("Error %d in encrypt(): %s\n", err, mcrypt_strerror(err));    
  }
  pop_n_elems(args);
  if(ivsize) {
    push_string(make_shared_binary_string(THIS->IV, ivsize));
    push_string(make_shared_binary_string(block_buffer, len));
    f_add(2);
  } else {
    push_string(make_shared_binary_string(block_buffer, len));
  }
}

static void f_decrypt(INT32 args) {
  int blocksize, datasize, needeblocks, padneeded;
  char *block_buffer, *indata;
  int err, ivsize = 0;
  MCRYPT td = THIS->td;
  if(!args || ARG(1).type != T_STRING) {
    Pike_error("Invalid or missing argument to decrypt(), expected string.\n");
  }
  indata = ARG(1).u.string->str;
  datasize = ARG(1).u.string->len << ARG(1).u.string->size_shift;
  if(mcrypt_enc_is_block_mode(td)) {
    blocksize =  mcrypt_enc_get_block_size(THIS->td);
    if( (datasize % blocksize) != 0 ) {
      Pike_error("Invalid length for indata. Should be a multiple of %d\n",
		 blocksize);
    }
  }
  if(THIS->runmode != MODE_DECRYPT) {
    if(mcrypt_enc_mode_has_iv(td)) {
      ivsize = mcrypt_enc_get_iv_size(THIS->td);
      if(datasize < ivsize) {
	Pike_error("Not enough data for random first block.\n");
      } 
      THIS->IV = realloc(THIS->IV, ivsize);
      if(!THIS->IV) {
	Pike_error("Out of memory.\n");
      }
      memcpy(THIS->IV, indata, ivsize);
      indata += ivsize;
      datasize -= ivsize;
    }
    generic_init(1);
    THIS->runmode = MODE_DECRYPT;
  }
  if(!datasize) {
    pop_n_elems(args);
    push_text("");
  }
  THIS->block_buffer = realloc(THIS->block_buffer, datasize);
  if(!THIS->block_buffer) {
    Pike_error("Failed to allocate block buffer!\n");
  }
  memcpy(THIS->block_buffer, indata, datasize);
  block_buffer = THIS->block_buffer;
  THREADS_ALLOW();
  err = mdecrypt_generic (td, block_buffer, datasize);
  THREADS_DISALLOW();
  if(err) {
    Pike_error("Error %d in decrypt(): %s\n", err, mcrypt_strerror(err));    
  }
  pop_n_elems(args);
  push_string(make_shared_binary_string(block_buffer, datasize));
}


static void f_mode(INT32 args) {
  char *name;
  pop_n_elems(args);
  name = mcrypt_enc_get_modes_name(THIS->td);
  push_text(name);
  mcrypt_free(name);
}

static void f_algorithm(INT32 args) {
  char *name;
  pop_n_elems(args);
  name = mcrypt_enc_get_algorithms_name(THIS->td);
  push_text(name);
  mcrypt_free(name);
}

static void f_block_size(INT32 args) {
  pop_n_elems(args);
  push_int(mcrypt_enc_get_block_size(THIS->td));
}

static void f_key_size(INT32 args) {
  pop_n_elems(args);
  push_int(mcrypt_enc_get_key_size(THIS->td));
}

static void f_key_sizes(INT32 args) {
  int sizes=0, *valid, i, j;
  pop_n_elems(args);
  valid = mcrypt_enc_get_supported_key_sizes(THIS->td, &sizes);
  if(!sizes && valid == NULL) {
    /* All sizes from 1 to mcrypt_enc_get_key_size is supported */
    j = mcrypt_enc_get_key_size(THIS->td); 
    for(i = 1; i <= j; i++) {
      push_int(i);
    }
    f_aggregate(j);
  } else {
    for(i = 0; i < sizes; i++) {
      push_int(valid[i]);
    }
    f_aggregate(sizes);
    mcrypt_free(valid);
  }
}

static void f_list_algorithms(INT32 args) {
  char **algs, **modes;
  int nalgs, nmodes, i, j, k;
  MCRYPT td;
  pop_n_elems(args);
  algs = mcrypt_list_algorithms(NULL, &nalgs);
  modes = mcrypt_list_modes(NULL, &nmodes);
  for(i = 0; i < nalgs; i++) {
    int found=0, maxkeysize=0, blocksize=0, nkeys=0, *keysizes;
    keysizes = NULL;
    push_text(algs[i]);
    push_text("modes");
    for (j = 0; j < nmodes; j++) {
      td = mcrypt_module_open(algs[i], NULL, modes[j], NULL);
      if(td != MCRYPT_FAILED) {
	push_text(modes[j]);
	if(!found) {
	  keysizes = mcrypt_enc_get_supported_key_sizes(td, &nkeys);
	  if(!nkeys)  maxkeysize = mcrypt_enc_get_key_size(td);
	  blocksize = mcrypt_enc_get_block_size(td);
	}
	found++;
	mcrypt_module_close(td);
      }
    }
    f_aggregate(found);
    push_text("key_sizes");
    if(!nkeys && keysizes == NULL) {
      for(k = 1; k <= maxkeysize; k++) {
	push_int(k);
      }
      f_aggregate(maxkeysize);
    } else {
      for(k = 0; k < nkeys; k++) {
	push_int(keysizes[k]);
      }
      f_aggregate(nkeys);
    }
    push_text("block_size");
    push_int(blocksize);
    f_aggregate_mapping(6);
  }
  f_aggregate_mapping(nalgs*2);
  mcrypt_free_p(algs, nalgs);
  mcrypt_free_p(modes, nmodes);
}

static void init_mcrypt_data(struct object *obj) {
  THIS->td = NULL;
  THIS->IV = NULL; 
  THIS->algo = NULL;
  THIS->mode = NULL;
  THIS->key = NULL;
  THIS->block_buffer = NULL;
  THIS->inited = 0;
  THIS->runmode = 0;
}
static void exit_mcrypt_data(struct object *obj) {
  if(THIS->td != NULL) {
    if(THIS->inited) {
      mcrypt_generic_deinit(THIS->td);
    }
    mcrypt_module_close(THIS->td);
  }
  if(THIS->IV != NULL) { free(THIS->IV); }
  if(THIS->block_buffer != NULL) { free(THIS->block_buffer); }
  if(THIS->algo != NULL) { free_string(THIS->algo); }
  if(THIS->mode != NULL) { free_string(THIS->mode); }
  if(THIS->key != NULL) { free_string(THIS->key); }
}

/*------------------------------------------------------------
 * Pike functions (standard -- build the module)
 *------------------------------------------------------------*/
void pike_module_init(void)
{
  start_new_program();
#ifdef ADD_STORAGE
  ADD_STORAGE(mcrypt_data);
#else
  add_storage(sizeof(mcrypt_data));
#endif
  add_function("create", f_create, "function(string,void|string:void)",0);
  
  add_function("key_size", f_key_size, "function(:int)",0);
  add_function("key_sizes", f_key_sizes, "function(:array(int))",0);
  add_function("block_size", f_block_size, "function(:int)",0);

  add_function("algorithm", f_algorithm, "function(:string)",0);
  add_function("mode", f_mode, "function(:string)",0);

  add_function("decrypt", f_decrypt, "function(string:string)",0);
  add_function("encrypt", f_encrypt, "function(string:string)",0);
  add_function("set_key", f_set_key, "function(string,string|void:void)",0);

  
  set_init_callback(init_mcrypt_data);
  set_exit_callback(exit_mcrypt_data);
  mcrypt_program = end_program();
  add_program_constant("mcrypt",mcrypt_program,0);
  add_function("list_algorithms", f_list_algorithms,
	       "function(:mapping)",0);
#ifdef PEXTS_VERSION
  pexts_init();
#endif
} /* pike_module_init */


void pike_module_exit(void)
{
  if ( mcrypt_program ) {
    free_program(mcrypt_program);
    mcrypt_program = NULL;
  } /* if */
} /* pike_module_exit */

#else
void pike_module_init(void) {
  add_integer_constant("LIBMCRYPT IS MISSING", 0, 0);
#ifdef PEXTS_VERSION
  pexts_init();
#endif
}
void pike_module_exit(void) {
}
#endif
