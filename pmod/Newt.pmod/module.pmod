// Dear Emacs, I humbly beg you to remember that I'm -*-Pike-*- code. Thanks.
//
// $Id$
//

class Screen
{
    inherit _Newt.ComponentBase;

    void finished()
    {
        _Newt.finished();
    }
    
    void create()
    {
        ::create(CLASS_SCREEN);
        _Newt.init();
    }

    void resume()
    {
        _Newt.resume();
    }

    void suspend()
    {
        _Newt.suspend();
    }

    void openWindow(int left, int top, int width, int height, string title) 
    {
        _Newt.openWindow(left, top, width, height, title);
    }

    void centeredWindow(int width, int height, string title)
    {
        _Newt.centeredWindow(width, height, title);
    }
    
    void pushHelpLine(string text)
    {
        _Newt.pushHelpLine(text);
    }

    void redrawHelpLine()
    {
        _Newt.redrawHelpLine();
    }
    
    void popHelpLine()
    {
        _Newt.popHelpLine();
    }

    void drawRootText(int left, int top, string text)
    {
        _Newt.drawRootText(left, top, text);
    }

    void popWindow()
    {
        _Newt.popWindow();
    }

    void refresh()
    {
        _Newt.refresh();
    }

    void cls()
    {
        _Newt.cls();
    }

    void resize(int|void redraw)
    {
        _Newt.resizeScreen(redraw ? 1 : 0);
    }

    void waitForKey()
    {
        _Newt.waitForKey();
    }

    void clearKeyBuffer()
    {
        _Newt.clearKeyBuffer();
    }

    void delay(int msecs)
    {
        _Newt.delay(msecs);
    }

    mapping(string:mapping(string:string)) defaultColors()
    {
        return _Newt.defaultColors();
    }

    void setColors(mapping(string:mapping(string:string)) colors)
    {
        _Newt.setColors(colors);
    }

    void bell()
    {
        _Newt.bell();
    }

    void cursorOff()
    {
#if constant(_Newt.cursorOff)
        _Newt.cursorOff();
#endif
    }

    void cursorOn()
    {
#if constant(_Newt.cursorOn)
        _Newt.cursorOn();
#endif
    }
};

private class Component
{
    inherit _Newt.ComponentBase;

    protected string my_text;
    
    string myText()
    {
        return my_text;
    }

    void create(int classID)
    {
        ::create(classID);
        my_text = "";
    }
};

class Button
{
    inherit Component;

    void create(int left, int top, string text, void|int compact)
    {
        ::create(CLASS_BUTTON);

        my_text = text;
        
        if (!compact)
            _Newt.button(left, top, my_text);
        else
            _Newt.compactButton(left, top, my_text);
    }
};

class Checkbox 
{
    inherit Component;
    
    void create(int left, int top, string text, void|string defVal, void|string seq)
    {
        ::create(CLASS_CHECKBOX);   

        my_text = text;
        if (defVal && !seq)
            _Newt.checkbox(left, top, text, defVal);
        else if (defVal && seq)
            _Newt.checkbox(left, top, text, defVal, seq);
        else
            _Newt.checkbox(left, top, text);
    }

    void setValue(string val)
    {
        _Newt.checkboxSetValue(val);
    }

    string getValue()
    {
        return _Newt.checkboxGetValue();
    }

    int selected()
    {
        string val = getValue();
        
        return (val && val[0] != ' ');
    }
}

class RadioButton
{
    inherit Component;

    void create(int left, int top, string text, void|int isdef, void|object(RadioButton) prev)
    {
        ::create(CLASS_RADIOBUTTON);
        my_text = text;

        if (isdef && !prev)
            _Newt.radiobutton(left, top, text, isdef ? 1 : 0);
        else if (isdef && prev)
            _Newt.radiobutton(left, top, text, isdef ? 1 : 0, prev);
        else
            _Newt.radiobutton(left, top, text);
    }
}

class Form
{
    inherit Component;

    private mapping(string:int) keynames =
    (["F1":_Newt.KEY_F1, "F2":_Newt.KEY_F2, "F3":_Newt.KEY_F3, "F4":_Newt.KEY_F4,
      "F5":_Newt.KEY_F5, "F6":_Newt.KEY_F2, "F7":_Newt.KEY_F2, "F8":_Newt.KEY_F8,
      "F9":_Newt.KEY_F9, "F10":_Newt.KEY_F10, "F11":_Newt.KEY_F11, "F12":_Newt.KEY_F12]);
    
    void create(string help, void|int flags)
    {
        ::create(CLASS_FORM);
        _Newt.form(0, help, 0);
    }

    object run()
    {
        return _Newt.formRun();
    }

    void draw()
    {
        _Newt.drawForm();
    }

    void add(object(Component) comp)
    {
        if (!comp)
            return;
        
        _Newt.formAddComponent(comp);
    }

    void setTimer(int msecs)
    {
        _Newt.formSetTimer(msecs);
    }

    void setSize()
    {
        _Newt.formSetSize();
    }

    object(Component) getCurrent()
    {
        return _Newt.formGetCurrent();
    }

    void setCurrent(object(Component) curr)
    {
        _Newt.formSetCurrent(curr);
    }
    
    void setBackground(int color)
    {
        _Newt.formSetBackground(color);
    }

    void setHeight(int height)
    {
        mapping(string:int) ssize = _Newt.getScreenSize();

        if (height > ssize->rows)
            return;

        _Newt.formSetHeight(height);
    }

    void setWidth(int width)
    {
        mapping(string:int) ssize = _Newt.getScreenSize();

        if (width > ssize->cols)
            return;

        _Newt.formSetWidth(width);
    }

    void addHotKey(int|string key)
    {
        int   hkey;
        
        if (stringp(key)) {
            if (keynames[key])
                hkey = keynames[key];
            else
                return;
        } else
            hkey = key;

        _Newt.formAddHotKey(hkey);
    }
};

class Entry 
{
    inherit Component;

    static private array(object|function) filters;
    static private int filter_engaged;
    
    void create(int left, int top, int width, string|void initVal, int|void flags)
    {
        ::create(CLASS_ENTRY);

        if (initVal && zero_type(flags)) {
            _Newt.entry(left, top, width, initVal);
        } else if (initVal && !zero_type(flags)) {
            _Newt.entry(left, top, width, initVal, flags);
        } else {
            _Newt.entry(left, top, width);
        }

        filters = ({});
        filter_engaged = 0;
    }

    string getValue()
    {
        return _Newt.entryGetValue();
    }

    string set(string text, int|void cursorAtEnd)
    {
        string ret = getValue();
        
        _Newt.entrySet(text, cursorAtEnd);

        return ret;
    }

    void setFlags(int flags, int sense)
    {
        _Newt.entrySetFlags(flags, sense);
    }

    static private int|string
    filterWrapper(string data, int|string ch, int cursor)
    {
        /* First one wins */
        foreach(filters, object|function filter) {
            if (objectp(filter) && filter->examine)
                return filter->examine(data, ch, cursor);
            else if (objectp(filter) && !filter->examine)
                throw(({"Invalid filter object. Missing the 'examine' method.", backtrace()}));
            else if (functionp(filter))
                return filter(data, ch, cursor);
            else
                throw(({"Invalid filter type.", backtrace()}));
        }
    }
    
    void setFilter(function|object filter, string|void data)
    {
        if (zero_type(filter))
            return;
        
        filters += ({filter});
        if (filter_engaged)
            return;

        if (!zero_type(data))
            _Newt.entrySetFilter(filterWrapper, data);
        else
            _Newt.entrySetFilter(filterWrapper);
        
        filter_engaged = 1;
    }

    void removeFilter(function|object filter)
    {
        if (zero_type(filter))
            return;

        if (!sizeof(filters))
            filters -= ({filter});

        if (!sizeof(filters))
            filter_engaged = 0;
    }
};

