#!/usr/local/roxen2/bin/pike-roxen

inherit AVS;

int main(int argc, string *argv)
{
  object index, file;
  string words;

  if (argc < 2)
    return 0;

  file = Stdio.File(argv[1], "r");
  if (!file)
    return 0;
  words = file->read();
  destruct(file);

  index = AVS.Index("/tmp/index", "rw");
  write((index->version() * "\n") + "\n\n");
  write("Indexing " +argv[1] + "\n");
  index->start_doc(argv[1]);
  index->add_words(words);
  index->end_doc();
  index->make_stable();
  while(index->compact());
  destruct(index);
}
