/*
 * NAME:	multicasr receiver
 * SYNOPSIS:	McastRcv <name> <multicast group> <port>
 * DESCRIPTION:	The program creates a datagram socket and joins a multicast
 *		group. It prints on the standard output all received message.
 * 
 * Translated to Pike by Daniel Serpell
 */

class Packet 
{
   string name;
   string msg;
   int lenght;
}

int Terminar = 0;
string UserName;
string mcGroup;
int Port;
Stdio.MultiCastUDP fd;
Stdio.File fin;

void packet_recive( mapping p )
{
   Packet pkt = Packet();
   
   write( sprintf("From %s:%d, %d bytes received\n", p->ip, p->port, strlen(p->data) ) );
   
   /* Transforma los datos recibidos a un paquete */
   sscanf( p->data, "%4c%16s%s", pkt->lenght, pkt->name, pkt->msg );
   
   write(sprintf("%s wrote : %s \n", pkt->name, pkt->msg));
   if ( pkt->msg == "STOP") 
   {
      /* Leave multicast group */
      fd->leave( mcGroup );
      exit(1);
   }
}

void ingresa_datos( mapping id, string s )
{
   Packet pkt = Packet();
   
   s[strlen(s)-1]=0;
   
   pkt->name   = UserName;
   pkt->msg    = s;
   pkt->lenght = strlen(s);
      
   /* Arma el paquete */
   string p = sprintf("%4c%16s%s", pkt->lenght, pkt->name, pkt->msg );
      
   write(sprintf("Sending %d bytes \"%s\"\n", strlen(p), pkt->msg));
   
   fd->send( mcGroup, Port, p );
}


int main( int argc, array(string) argv )
{
   
   if (argc != 4) {
      Stdio.stderr->write( "Usage : " + argv[0] + " <name> <multicast group> <port>\n");
      return 0;
   }
   UserName = argv[1];
   mcGroup = argv[2];
   Port    = (int)argv[3];
   
   fd = Stdio.MultiCastUDP();
   
   fd->bind( Port );
   
   /* Join Multicast Group */
   fd->join( mcGroup );
   
   /* Set socket to send datagrams */
   fd->setLoopback( 1 );
   fd->setTTL( 5 );
   
   fd->set_nonblocking( packet_recive );
   
   fin = Stdio.File("stdin");
   
   fin->set_nonblocking(ingresa_datos,0,0 );
   return -1;
   
   
}
