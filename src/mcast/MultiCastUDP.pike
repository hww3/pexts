/* Parte escrita en Pike del módulo MultiCastUDP.
 * Copiado de la clase UDP de Pike.
 * La idea es copiarlo en ${PIKE_DIR}/lib/modules/Stdio.pmod/
 */

   inherit mcast.MultiCastUDP;

   private static array extra=0;
   private static function callback=0;

   object set_nonblocking(mixed ...stuff)
   {
      if (stuff!=({})) 
	 set_read_callback(@stuff);
      return _set_nonblocking();
   }

   object set_read_callback(function f,mixed ...ext)
   {
      extra=ext;
      callback=f;
      _set_read_callback(_read_callback);
      return this_object();
   }
   
   private static void _read_callback()
   {
      mapping i;
      if (i=read())
	callback(i,@extra);
   }
