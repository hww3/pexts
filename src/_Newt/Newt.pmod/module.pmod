class Screen
{
    inherit _Newt.ComponentBase;

    void finished()
    {
        _Newt.finished();
    }
    
    void create()
    {
        ::create(0x0C);
        _Newt.init();
    }
};
