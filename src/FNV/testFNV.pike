#! /usr/bin/env pike

void main (int argc, array argv) {
  if (argc != 2) {
    write ("usage: " + argv[0] + " <string>\n");
    exit (1);
  }
  
  string str = argv[1];

  int h32 = .FNV.fnv1_32()->hash(str);
  write (sprintf ("FNV hash32 ('" + str + "')= 0x%x         (%d)\n", h32, h32));
  int h64 = .FNV.fnv1_64()->hash(str);
  write (sprintf ("FNV hash64 ('" + str + "')= 0x%x (%d)\n", h64, h64));
}
