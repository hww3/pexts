#!/usr/bin/pike -M./

int main()
{
    object n = Newt.Screen();

    n->cls();
    n->drawRootText(0, 0, "SOME TEXT!!!!!!!!!!!!!!!!!!");

    n->openWindow(20, 10, 50, 20, "Hello world");
    
    object form = Newt.Form("Some help text");
    form->add(Newt.Button(2, 2, "Button"));
    form->add(Newt.Button(2, 7, "Compact", 1));
    form->add(Newt.Checkbox(15, 2, "CB", " ", " 123"));
    form->add(Newt.RadioButton(15, 7, "RB", 1));
    
    object(Newt.Button) ret = form->run();
    
    n->finished();    
    
    write(sprintf("Finished by the '%s' button\n", ret->myText()));
    
    return 0;
}
