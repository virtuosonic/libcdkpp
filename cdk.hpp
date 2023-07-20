/******************************
Name:           cdk.hpp
Description:    RAII C++ wrapper for libcdk
Date:           2023-JUL-19
Author:         Gabriel Espinoza (virtuosonic @ github)
License:        MIT
*/

#pragma once

#include <cdk/cdk.h>

#include <memory>

namespace cdk {


auto string2charptr = [](const string_view s)
{
	return const_cast<char*>(s.data());
};

using StringList = std::vector<string_view>;

vector<char*> transformStringList(const StringList &v)
{
    vector<char*> vc;
    transform(v.begin(),v.end(),back_inserter(vc),string2charptr);
    return vc;
}

/**
 * Screen object that manages its child widgets.
*/
class screen {
    using screenptr = std::unique_ptr<CDKSCREEN,destroyCDKScreen>;
    screenptr _ptr;
    static bool atexit_installed{false};
    friend class label;
public:
    /**
     * Constructor.
     * Creates an object in a curses WINDOW,
     * you can pass initscr() to use the full
     * terminal. It calls initCDKScreen and 
     * install an atexit handler
    */
    screen(WINDOW *cursesWindow){
        _ptr = initCDKScreen(cursesWindow);
        initCDKColor();
        if (!atexit_installed){
            atexit(endCDK); 
        }
    }
    /**
     * Erase screen.
     * Erases all of the widgets which 
     * are currently associated to the given screen.  
     * This does NOT destroy the widgets.
    */
    void erase() { 
        eraseCDKScreen(_ptr.get());
    }
    /**
     * Redraws all of the widgets 
     * which are currently associated 
     * to this screenobject.
    */
    void refresh() {
        refreshCDKScreen(_ptr.get());
    }
}

struct point
{
    int x,y;
};

struct drawing_options
{
    bool box{false},shadow{false};
};

struct move_options{
    bool relative{false},refresh{false};
}

/**
 * A managed curses label widget.
*/
class label {
    using labelptr = std::unique_ptr<CDKLABEL,destroyCDKLabel>;
    labelptr _ptr;
public:
    /**
     * Default constructor.
     * Creates an empty object, dont call
     * member functions without assigning
     * an initialized object
    */
    label() = default;

    label(screen& parent,point p,StringList message, drawing_options opt)
    {
        _ptr = newCDKLabel(parent._ptr.get(),
                    p.x,p.y,
                    transformStringList(message),
                    opt.box,opt.shadow);
    }
    /**
     * Draws the label widget on the screen.  If the box parameter is true, the widget is drawn with a box.
    */
    draw(boolean box=false)
    {
        drawCDKLabel(_ptr.get(),box)
    }
    /**
     * Removes the widget from the screen.  This does NOT destroy the widget.
    */
    erase()
    {
        eraseCDKLabel(_ptr.get());
    }
    /**
     * Returns true if the widget will be drawn with a box around it.
    */
    bool getBox()
    {
        return getCDKLabelBox(_ptr.get());
    }
    /**
     * Returns the contents of the label widget.  
     * The parameter messageLines points to the location 
     * which will be given the number of message-lines.
    */
    StringList getMessage()
    {
        StringList v;
        int linesCount=0;
        auto msg = getCDKLabelMessage(_ptr.get(),&linesCount);
        for(int i = 0 ; i< linesCount;++i)
        {
            v.push_back(msg[i]);
        }
        return v;
    }
    /**
     * moves the widget to the given point.

            •   The parameters p.x and p.y are the new position of the widget.

                The parameter p.x may be an integer or one of the pre-defined values TOP, BOTTOM, and CENTER.

                The parameter p.y may be an integer or one of the pre-defined values LEFT, RIGHT, and CENTER.

            •   The parameter relative states whether the p.x/p.y pair is a relative move or an absolute move.
    */
    void move(point p,move_options o)
    {
        moveCDKLabel(_ptr.get(),
                p.x,p.y,
                o.relative,o.refresh);
    }
    /**
     * Allows the user to move the widget around the screen 
     * via the cursor/keypad keys.  
     * See cdk_position (3) for key bindings.
    */
    void position()
    {
        positionCDKLabel(_ptr.get());
    }
    /**
     * allows the user to change the contents of the label widget.  
     * The parameters are the same as the newCDKLabel.
    */
    void set(StringList message,bool box)
    {
        auto v = transformStringList(message);
        setCDKLabel(_ptr.get(),
            v.data(),v.size()
            box);
    }
    /**
     * Sets the background attribute of the widget.  
     * The parameter attribute is a curses attribute, 
     * e.g., A_BOLD.
    */
    void setBackgroundAttrib(chtype attribute)
    {
        setCDKLabelBackgroundAttrib(_ptr.get(),attribute);
    }
    /**
     * Sets the background color of the widget.  
     * The parameter color is in the format of 
     * the Cdk format strings.  
     * For more information see cdk_display (3).
    */
    void setBackgroundColor(const char * color)
    {
        setCDKLabelBackgroundColor(_ptr.get(),color)
    }
    /**
     * Sets whether the widget will be drawn with a box around it.
    */
    void setBox(bool box)
    {
        setCDKLabelBox(_ptr.get(),box);
    }
    /**
     * Sets the attribute of the box.
    */
    void setBoxAttribute(chtype character)
    {
        setCDKLabelBoxAttribute(_ptr.get(),character);
    }
    /**
     * Sets the horizontal drawing character for the box to the given character.
    */
    void setHorizontalChar(chtype character)
    {
        setCDKLabelHorizontalChar(_ptr.get(),character);
    }
    /**
     * Sets the lower left hand corner of the widget's box to the given character.
    */
    void setLLChar(chtype character)
    {
        setCDKLabelLLChar(_ptr.get(),character);
    }
    /**
     * Sets the lower right hand corner of the widget's box to the given character.
    */
    void setLRChar(chtype character)
    {
        setCDKLabelLRChar(_ptr.get(),character)
    }
    /**
     * This sets the contents of the label widget.
    */
    void setMessage(StringList message)
    {
        auto v = transformStringList(message);
        setCDKLabelMessage(_ptr.get(),
            v.data(),v.size());
    }
    /**
     * Sets the upper left hand corner of the widget's box to the given character.
    */
    void setULChar(chtype character)
    {
        setCDKLabelULChar(_ptr.get(),character);
    }
    /**
     * Sets the upper right hand corner of the widget's box to the given character.
    */
    void setURChar(chtype character)
    {
        setCDKLabelURChar(_ptr.get(),character);
    }
    /**
     * Sets the vertical drawing character for the box to the given character.
    */
    void setVerticalChar(chtype character)
    {
        setCDKLabelVerticalChar(_ptr.get(),character);
    }
    /**
     * Waits for a user to press a key.  
     * The label parameter is the pointer to a label widget, 
     * and key is the key to wait for.  
     * If no specific key is desired, use (char)0.
    */
    char wait(char key)
    {
        waitCDKLabel(_ptr.get(),char key);
    }

}

}//namespace cdk