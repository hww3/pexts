/*
 * mcast.c: Implementa un objeto pike para el uso de sockets multicast.
 * 
 * Daniel Serpell C.
 * Octubre del 2000.
 * 
 * Teoría:
 *    La idea es extender el objeto Stdio.UDP agregando funcionalidad
 *    necesaria para multicast.
 * Funciones:
 *    join(string group):	Se une al grupo Multicast "group".
 *    leave(string group):	Deja en el grupo Multicast "group".
 *    setLoopback(int s):	Activa el Loopback (0 ó 1).
 *    setTTL(int s):		Fija el TTL.
 *    setInterface(string if):	Fija el IP de la interfáz a utilizar.
 * 				Si no se llama, utiliza INADDR_ANY.
 * 
 */

/* Pike includes */
#include "global.h"
#include "interpret.h"
#include "svalue.h"
#include "stralloc.h"
#include "array.h"
#include "pike_macros.h"
#include "program.h"
#include "stralloc.h"
#include "object.h"
#include "pike_types.h"
#include "threads.h"
#include "dynamic_buffer.h"

#include "mcast_config.h"

/* System includes */
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

/* Esta estructura proviene del código de "udp.c".
 * Si llega a cambiar, no funciona más.... */
struct udp_storage
{
   int fd;
   int my_errno;
   struct svalue read_callback;
};

/* Estructura con los datos internos al objeto */
struct mcast_storage 
{
   uint32_t  if_addr;
   
   struct udp_storage udp; /* Debe estar al final */
};


/* Programa padre */
static struct program *stdio_udp; 

/* Macro para acceder a los datos internos */
#undef THIS
#define THIS ((struct mcast_storage *)(Pike_fp->current_storage))
#define UDP (&(THIS->udp))
#define FD (UDP->fd)

static void mcast_join(INT32 args)
{
   uint32_t mc_addr;
   struct ip_mreq mreq;
   
   if(args!=1)
      Pike_error("mcast->join(): número de argumentos inválido\n");
   if(Pike_sp[-1].type!=T_STRING)
      Pike_error("mcast->join(): Tipo del argumento inválido\n");
   
   /* Verifica el estado del socket */
   if( FD < 0 )
     /* El objeto no está inicializado... */
     Pike_error("mcast->join(): Port not bound! (call \"bind\" first)\n");
   
   
   mc_addr = inet_addr(Pike_sp[-1].u.string->str);
   if(mc_addr == -1)
     Pike_error("mcast->join(): Invalid mcast group\n");
   
   mreq.imr_multiaddr.s_addr = mc_addr;
   mreq.imr_interface.s_addr = THIS->if_addr;
   if ( setsockopt(FD,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *) &mreq,
		   sizeof(mreq)) == -1 )
   {
      UDP->my_errno = errno;
      Pike_error("mcast->join(): error in joining group (%s)\n",strerror(errno));
   }
   
   pop_n_elems(args);
}

static void mcast_leave(INT32 args)
{
   uint32_t mc_addr;
   struct ip_mreq mreq;

   if(args!=1)
      Pike_error("mcast->leave(): número de argumentos inválido\n");
   if(Pike_sp[-1].type!=T_STRING)
      Pike_error("mcast->leave(): Tipo del argumento inválido\n");
   
   /* Verifica el estado del socket */
   if( FD < 0 )
     /* El objeto no está inicializado... */
     Pike_error("mcast->leave(): Port not bound! (call \"bind\" first)\n");
   
   mc_addr = inet_addr(Pike_sp[-1].u.string->str);
   if(mc_addr == -1)
     Pike_error("mcast->leave(): Invalid mcast group\n");
   
   mreq.imr_multiaddr.s_addr = mc_addr;
   mreq.imr_interface.s_addr = THIS->if_addr;
   if( setsockopt(FD,IPPROTO_IP,IP_DROP_MEMBERSHIP,
		  (char *) &mreq,sizeof(mreq)) == -1 )
   {
      UDP->my_errno = errno;
      Pike_error("mcast->leave(): error in leaving group (%s)\n",strerror(errno));
   }
   
   pop_n_elems(args);
}

static void mcast_setTTL(INT32 args)
{
   int ttl;
   
   if( args != 1 )
      Pike_error("mcast->setTTL(): número de argumentos inválido\n");
   if(Pike_sp[-1].type != T_INT)
      Pike_error("mcast->setTTL(): Tipo del argumento inválido\n");
   
   ttl = Pike_sp[-args].u.integer;
   
   if( setsockopt(FD ,IPPROTO_IP,IP_MULTICAST_TTL,(char *) &ttl,
		  sizeof(u_char)) == -1 )
   {
      UDP->my_errno = errno;
      Pike_error("mcast->setTTL(): error in setting TTL (%s)\n",strerror(errno));
   }
   pop_n_elems(args);
}

static void mcast_loopback(INT32 args)
{
   int loop;
   
   if( args != 1 )
      Pike_error("mcast->setLoopback(): número de argumentos inválido\n");
   if(Pike_sp[-1].type != T_INT)
      Pike_error("mcast->setLoopback(): Tipo del argumento inválido\n");
   
   loop = Pike_sp[-args].u.integer;
   
   if( setsockopt(FD ,IPPROTO_IP,IP_MULTICAST_LOOP,(char *) &loop,
		  sizeof(u_char)) == -1 )
   {
      UDP->my_errno = errno;
      Pike_error("mcast->setLoopback(): error in setting loopback (%s)\n",strerror(errno));
   }
   pop_n_elems(args);
}

static void mcast_setif(INT32 args)
{
   uint32_t ifaddr;
   
   if(args!=1)
      Pike_error("mcast->setInterface(): número de argumentos inválido\n");
   if(Pike_sp[-1].type!=T_STRING)
      Pike_error("mcast->setInterface(): Tipo del argumento inválido\n");
   
   
   ifaddr = inet_addr(Pike_sp[-1].u.string->str);
   if(ifaddr == -1)
     Pike_error("mcast->setInterface(): Invalid interface address\n");
   
   THIS->if_addr = ifaddr;
   
   pop_n_elems(args);
}

static void init_mcast(struct object *ignored)
{
   MEMSET(THIS, 0, sizeof(struct mcast_storage) - sizeof(struct udp_storage));
   THIS->if_addr = INADDR_ANY;
}

void pike_module_exit(void) {}

void pike_module_init(void)
{
   struct svalue sv;
   
   /* Comienzo de la nueva clase */
   start_new_program();
   
   /* Agrega espacio para los datos internos */
   low_add_storage(sizeof(struct mcast_storage) - sizeof(struct udp_storage),
		   ALIGNOF(struct mcast_storage),0);

   /* Hereda Stdio.UDP */
   /* NOTA IMPORTANTE
    * Aparentemente no se puede heredar un objeto
    * escrito en Pike desde aquí... luego, heredo
    * el objeto básico (nativo) */

   /* Resuelve el objeto (encuentra el archivo) */
   push_text("files.UDP");
   SAFE_APPLY_MASTER("resolv",1);
   if(Pike_sp[-1].type != T_FUNCTION)
     Pike_error("Error al resolver Stdio.UDP!\n");
   
   /* Obtiene el programa */
   stdio_udp = program_from_function(&Pike_sp[-1]);
   pop_n_elems(1);
   
   /* Hereda */
   sv.type = T_PROGRAM;
   sv.subtype = 0;
   sv.u.program = stdio_udp;
   do_inherit( &sv, 0, 0);
   
   /* Agrega los métodos */
   ADD_FUNCTION("join",mcast_join,tFunc(tStr,tVoid),0);
   ADD_FUNCTION("leave",mcast_leave,tFunc(tStr,tVoid),0);
   ADD_FUNCTION("setLoopback",mcast_loopback,tFunc(tInt,tVoid),0);
   ADD_FUNCTION("setTTL",mcast_setTTL,tFunc(tInt,tVoid),0);
   ADD_FUNCTION("setInterface",mcast_setif,tFunc(tStr,tVoid),0);
   
   /* Llama a "init_mcast" antes de crear los objetos */
   set_init_callback(init_mcast);
   
   end_class("MultiCastUDP",0);
}
