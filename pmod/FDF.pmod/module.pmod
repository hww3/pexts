// Dear Emacs, I humbly beg you to remember that I'm -*-Pike-*- code. Thanks.
//
// $Id$
//
inherit _FDF;

class File
{
    inherit _FDF.File;

    array(string)|int getAllFieldNames() 
    {
        string|int      fn;
        array(string)   ret = ({});
        
        fn = NextFieldName();
        while (stringp(fn)) {
            write(sprintf("fn: %O\n", fn));
            ret += ({fn});
            fn = NextFieldName(fn);
        }

        if (sizeof(ret))
            return ret;

        return 0;
    }
    
    void create(string|void path, int|void howMany)
    {
        ::create(path, howMany);
    }
};
