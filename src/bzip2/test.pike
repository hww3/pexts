#!/usr/bin/pike7-cvs -M./

//
// Test script
//
// $Id$
//

string infile = "bzip2.so";
string packedfile = "output.bin.bz2";
string outfile = "output.bin";

int main()
{
    object f = Stdio.File(infile, "r");
    string orig = f->read();
    string packed, unpacked;
    
    f->close();

    object d = bzip2.deflate(9);
    packed = d->deflate(orig);
    f->open(packedfile, "wct");
    f->write(packed);
    f->close();
    
    float ratio = ((float)sizeof(packed)/(float)sizeof(orig));
    
    
    object u = bzip2.inflate(0);
    unpacked = u->inflate(packed);
    f->open(outfile, "wct");
    f->write(unpacked);
    f->close();

    write(sprintf("      Original file: %s (%u bytes)\n",
                  infile, sizeof(orig)));
    write(sprintf("    Compressed file: %s (%u bytes)\n", 
                  packedfile, sizeof(packed)));
    write(sprintf("    Copy of original\n"
                  "after decompression: %s (%u bytes)\n",
                  outfile, sizeof(unpacked)));
    write(sprintf("  Compression ratio: %.1f%%\n", ratio * 100));
    
    return 1;
}
