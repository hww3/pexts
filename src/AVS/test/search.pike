#!/usr/local/roxen2/bin/pike-roxen

inherit AVS;

void st(object i) {
  object s  = i->search(i->SEARCH_SIMPLE, "hello world");
  sleep(1);
}

int main(int argc, string *argv)
{
  object index, search;
  string docid, data;
  float relevance;
  array date;

  if (argc < 2)
    return 0;

  index = AVS.Index("/tmp/index/", "r");
  write((index->version() * "\n") + "\n\n");
  search = index->search(index->SEARCH_BOOLEAN, argv[1]);
  write("Documents found: " + search->docsfound() + "\n");
  write("Documents returned: " + search->docsreturned() + "\n");
  write("Numner of terms: " + search->termcount() + "\n");
  /*  write("Index version: " + search->get_version() + "\n\n");*/
  int cnt = search->docsreturned();
  if(cnt > 50) cnt = 50;
  for(int i; i < cnt; i++)
  {
    search->get_result(i);
    docid = search->get_docid();
    relevance = search->get_relevance();
    date = search->get_date();
    data = search->get_data();
    write(docid + ":\n" + sprintf("%O\n", decode_value(data)) +
	  "\n (" + relevance + ") "+date[0]+"-"+date[1]+"-"+date[2]+"\n");
  }
  destruct(search);

  destruct(index);
}
