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
#include <string_view>
#include <vector>
#include <algorithm>
#include <functional>

namespace cdk
{


auto string2charptr = [](const std::string_view s)
{
	return const_cast<char*>(s.data());
};

using StringList = std::vector<std::string_view>;

std::vector<char*> transformStringList(const StringList &v)
{
	std::vector<char*> vc;
	std::transform(v.begin(),v.end(),back_inserter(vc),string2charptr);
	return vc;
}

struct widget
{
	void* _vptr{nullptr};
	EObjectType type{vNULL};
};
/**
 * Screen object that manages its child widgets.
*/
class screen
{
	struct deleter
	{
		void operator ()(CDKSCREEN* p)
		{
			destroyCDKScreen(p);
		}
	};

	using screenptr = std::unique_ptr<CDKSCREEN,deleter>;
	screenptr _ptr;
	//static bool atexit_installed{false};
	friend class label;
	friend class button;
public:
	/**
	 * Constructor.
	 * Creates an object in a curses WINDOW,
	 * you can pass initscr() to use the full
	 * terminal. It calls initCDKScreen and
	 * install an atexit handler
	*/
	screen(WINDOW *cursesWindow)
	{
		_ptr = screenptr(initCDKScreen(cursesWindow));
		initCDKColor();
		//if (!atexit_installed){
		//atexit(endCDK);
		//}
	}
	~screen()
	{
		endCDK();
	}
	/**
	 * Erase screen.
	 * Erases all of the widgets which
	 * are currently associated to the given screen.
	 * This does NOT destroy the widgets.
	*/
	void erase()
	{
		eraseCDKScreen(_ptr.get());
	}
	/**
	 * Redraws all of the widgets
	 * which are currently associated
	 * to this screenobject.
	*/
	void refresh()
	{
		refreshCDKScreen(_ptr.get());
	}
	/**
	 * @brief lowerObject.
	 * Has the opposite effect of the raiseCDKObject function call.
	 * @param w pointer to widget
	 */
	void lowerObject(widget* w)
	{
		lowerCDKObject(w->type,w->_vptr);
	}
	/**
	 * @brief raiseObject.
	 * raises the widget to the top of the screen.
	 * If there are any widgets which overlap the given object
	 * when a refresh is done, calling this function has the
	 * effect of raiding the object so no other widgets obstruct it.
	 * @param w pointer to the object
	 */
	void raiseObject(widget* w)
	{
		raiseCDKObject(w->type,w->_vptr);
	}
	/**
	 * @brief registerObject.
	 * Is called automatically when a widget is created.
	 * If for some reason an object does get unregistered,
	 * by calling unregisterObject,
	 * the widget can be registered again by calling this function.
	 */
	void registerObject(widget* w)
	{
		registerCDKObject(_ptr.get(),w->type,w->_vptr);
	}
	/**
	 * @brief unregisterObject.
	 * removes  the  widget  from  the screen.
	 * This does NOT destroy the object,
	 * it removes the widget from any further
	 * refreshes by the function refresh.
	 */
	void unregisterObject(widget* w)
	{
		unregisterCDKObject(w->type,w->_vptr);
	}

};

struct point
{
	int x,y;
};

struct drawing_options
{
	bool box{false},shadow{false};
};

struct move_options
{
	bool relative{false},refresh{false};
};

/**
 * A managed curses label widget.
*/
class label :public widget
{
	struct deleter
	{
		void operator()(CDKLABEL* p)
		{
			destroyCDKLabel(p);
		}
	};
	using labelptr = std::unique_ptr<CDKLABEL,deleter>;
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
		auto v = transformStringList(message);
		_ptr = labelptr(newCDKLabel(parent._ptr.get(),
									p.x,p.y,
									v.data(),v.size(),
									opt.box,opt.shadow));
		_vptr = _ptr.get();
	}
	/**
	 * Draws the label widget on the screen.  If the box parameter is true, the widget is drawn with a box.
	*/
	void draw(boolean box=false)
	{
		drawCDKLabel(_ptr.get(),box);
	}
	/**
	 * Removes the widget from the screen.  This does NOT destroy the widget.
	*/
	void erase()
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
		for(int i = 0 ; i< linesCount; ++i)
		{
			v.push_back(reinterpret_cast<char*>(msg[i]));
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
					v.data(),v.size(),
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
		setCDKLabelBackgroundColor(_ptr.get(),color);
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
		setCDKLabelLRChar(_ptr.get(),character);
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
		return waitCDKLabel(_ptr.get(),key);
	}
	EObjectType type{vLABEL};

};

/**
 * @brief create and manage a curses button widget.
 */
class button : public widget
{
	struct deleter
	{
		void operator()(CDKBUTTON* p)
		{
			destroyCDKButton(p);
		}
	};
	using buttonptr = std::unique_ptr<CDKBUTTON,deleter>;
	buttonptr _ptr;
public:
	button() = default;
	/**
	 * @brief Creates a new buttonbox widget.
	 * @param s parent screen
	 * @param p
	 * @param message
	 * @param o
	 */
	button(screen& s,point p, std::string_view message,tButtonCallback cb,drawing_options o)
	{
		_ptr = buttonptr(newCDKButton(s._ptr.get(),
									  p.x,p.y,
									  message.data(),
									  cb,
									  o.box,o.shadow));
		_vptr = _ptr.get();
	}
	/**
	 * @brief activates the button widget and lets the user  interact  with  the
			widget.
	 * @param actions If  the actions parameter is passed with a non-NULL value, the
				characters in the array will be injected into the widget.
	 * @return If  the  character  entered into this widget is RETURN or TAB then
			this function will return a value from 0 to the number of  buttons
			-1, representing the button selected.  It will also set the widget
			data exitType to vNORMAL.

			If the character entered into this widget was ESCAPE then the wid‐
			get will return a value of -1 and the widget data exitType will be
			set to vESCAPE_HIT.
	 */
	int activate(chtype * actions)
	{
		return activateCDKButton (_ptr.get(),actions);
	}
	/**
	 * @brief draws the button widget on the screen.
	 * @param box If the box parameter is true, the widget is drawn with a box.
	 */
	void draw (bool box)
	{
		drawCDKButton (_ptr.get(),box);
	}
	/**
	 * @brief erase removes  the  widget  from  the screen.  This does NOT destroy the
			widget.
	 */
	void erase()
	{
		eraseCDKButton(_ptr.get());
	}
	/**
	 * @brief getBox
	 * @return returns true if the widget will be drawn with a box around it.
	 */
	bool getBox()
	{
		return getCDKButtonBox (_ptr.get());
	}
	/**
	 * @brief getMessage
	 * @return the contents of the button widget.
	 */
	std::string_view getMessage()
	{
		auto msg = getCDKButtonMessage(_ptr.get());
		return {reinterpret_cast<char*>(msg)};
	}
	/**
	 * @brief inject injects a single character into the widget.
	 * @param input the character to  inject  into  the widget.
	 * @return The  return  value  and side-effect (setting the widget data exit‐
			Type) depend upon the injected character:

			RETURN or TAB
				   this function returns 0, representing the button  selected.
				   The widget data exitType is set to vNORMAL.

			ESCAPE the  function  returns -1.  The widget data exitType is set
				   to vESCAPE_HIT.

			Otherwise
				   unless modified by  preprocessing,  postprocessing  or  key
				   bindings,  the  function returns -1.  The widget data exit‐
				   Type is set to vEARLY_EXIT.
	 */
	int inject(chtype input)
	{
		return injectCDKButtonbox (_ptr.get(),input);
	}
	void set(std::string_view message,bool box)
	{
		setCDKButton(_ptr.get(),message.data(),box);
	}
	void setBackgroundAttrib(chtype attribute)
	{
		setCDKButtonBackgroundAttrib (_ptr.get(),attribute);
	}
	void setBackgroundColor(const char* color )
	{
		setCDKButtonBackgroundColor(_ptr.get(),color);
	}
	void setBox(bool box)
	{
		setCDKButtonBox(_ptr.get(), box);
	}
	void setBoxAttribute(chtype c)
	{
		setCDKButtonBoxAttribute(_ptr.get(),c);
	}
	void setHorizontalChar(chtype c)
	{
		setCDKButtonHorizontalChar(_ptr.get(),c);
	}
	void setLLChar(chtype c)
	{
		setCDKButtonLLChar(_ptr.get(),c);
	}
	void setLRChar (chtype c)
	{
		setCDKButtonLRChar(_ptr.get(),c);
	}
	void setMessage(std::string_view message)
	{
		setCDKButtonMessage(_ptr.get(),message.data());
	}
	void setULChar(chtype c)
	{
		setCDKButtonULChar(_ptr.get(),c);
	}
	void setURChar(chtype c)
	{
		setCDKButtonURChar(_ptr.get(),c);
	}
	void setVerticalChar(chtype c)
	{
		setCDKButtonVerticalChar(_ptr.get(),c);
	}
	/**
	 * @brief move moves to the given position.
	 * @param p new position of the  wid‐
				get.
	 * @param o move_options
	 */
	void move(point p,move_options o)
	{
		moveCDKButton(_ptr.get(),
					  p.x,p.y,
					  o.relative,o.refresh);
	}
	void position()
	{
		positionCDKButton(_ptr.get());
	}
	///man cdk_button(3) documents this function but it
	/// appears that it doesn't exist
//	void wait(char key)
//	{
//		waitCDKButton(_ptr.get(),key);
//	}
	EObjectType type{vBUTTON};

};

}//namespace cdk
