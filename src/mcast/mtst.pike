
void pa( array(string) a )
{
   string i;
   
   write("({\n");
   foreach(a,i) write( "\"" + i + "\"\n" );
   
   write("})\n");
}



int main(int argc, array(string) argv)
{
   object o = MultiCastUDP();
   
   o->bind((int)argv[2]);
   o->join(argv[1]);
   
   while(1)
   {
      mapping s = o->read();
      write("Rec from: " + s->ip + ":" + s->port + " data='" + s->data + "'\n");
   }

   return 0;
}
