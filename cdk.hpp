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

StringList transformCharPtrPtr(char** str,int size)
{
	StringList l;
	for (int i = 0; i< size ; ++i)
	{
		l.push_back(str[i]);
	}
	return l;
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
	static bool atexit_installed;
	friend class label;
	friend class button;
	friend class text_entry;
	friend class alpha_list;
	friend class calendar;
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
		if (!atexit_installed){
			atexit(endCDK);
			atexit_installed = true;
		}

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
	static void lowerObject(widget& w)
	{
		lowerCDKObject(w.type,w._vptr);
	}
	/**
	 * @brief raiseObject.
	 * raises the widget to the top of the screen.
	 * If there are any widgets which overlap the given object
	 * when a refresh is done, calling this function has the
	 * effect of raising the object so no other widgets obstruct it.
	 * @param w pointer to the object
	 */
	static void raiseObject(widget& w)
	{
		raiseCDKObject(w.type,w._vptr);
	}
	/**
	 * @brief registerObject.
	 * Is called automatically when a widget is created.
	 * If for some reason an object does get unregistered,
	 * by calling unregisterObject,
	 * the widget can be registered again by calling this function.
	 */
	void registerObject(widget& w)
	{
		registerCDKObject(_ptr.get(),w.type,w._vptr);
	}
	/**
	 * @brief unregisterObject.
	 * removes  the  widget  from  the screen.
	 * This does NOT destroy the object,
	 * it removes the widget from any further
	 * refreshes by the function refresh.
	 */
	static void unregisterObject(widget& w)
	{
		unregisterCDKObject(w.type,w._vptr);
	}

};
bool screen::atexit_installed=false;

/**
 * @brief The point struct. Used to pass position on the screen.
 */
struct point
{
	/**
	 * @brief Position in x axis. May be an integer or one of the pre-defined values TOP, BOTTOM, and CENTER.
	 */
	int x;
	/**
	 * @brief Position in x axis. May be an integer or one of the pre-defined values LEFT, RIGHT, and CENTER.
	 */
	int y;
};

struct widget_size
{
	int width;
	int height;
};

/**
 * @brief Options passed to draw functions.
 */
struct drawing_options
{
	/**
	 * @brief Whether to draw a box around the widget.
	 */
	bool box{false};
	/**
	 * @brief Whether to draw a shadow around the widget.
	 */
	bool shadow{false};
};

/**
 * @brief Options passed to move functions.
 */
struct move_options
{
	bool relative{false};
	bool refresh{false};
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
	 * @brief Default constructor.
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
	 * @brief Draws the label widget on the screen.
	 * @param box if true, the widget is drawn with a box.
	*/
	void draw(boolean box=false)
	{
		drawCDKLabel(_ptr.get(),box);
	}
	/**
	 * @brief Removes the widget from the screen.  This does NOT destroy the widget.
	*/
	void erase()
	{
		eraseCDKLabel(_ptr.get());
	}
	/**
	 * @brief Returns true if the widget will be drawn with a box around it.
	*/
	bool getBox()
	{
		return getCDKLabelBox(_ptr.get());
	}
	/**
	 * @brief Returns the contents of the label widget.
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
	 * @brief moves the widget to the given point.
	 * @param p The parameters p.x and p.y are the new position of the widget.

				- The parameter p.x may be an integer or one of the pre-defined values TOP, BOTTOM, and CENTER.

				- The parameter p.y may be an integer or one of the pre-defined values LEFT, RIGHT, and CENTER.
	 * @param o The parameter o.relative states whether the p.x/p.y pair is a relative move or an absolute move.
	*/
	void move(point p,move_options o)
	{
		moveCDKLabel(_ptr.get(),
					 p.x,p.y,
					 o.relative,o.refresh);
	}
	/**
	 * @brief Allows the user to move the widget around the screen
	 * via the cursor/keypad keys.
	 * @sa cdk_position (3) for key bindings.
	*/
	void position()
	{
		positionCDKLabel(_ptr.get());
	}
	/**
	 * @brief Allows the user to change the contents of the label widget.
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
	 * @brief  Sets the background attribute of the widget.
	 * @param attribute is a curses attribute,
	 * e.g., A_BOLD.
	*/
	void setBackgroundAttrib(chtype attribute)
	{
		setCDKLabelBackgroundAttrib(_ptr.get(),attribute);
	}
	/**
	 * @brief Sets the background color of the widget.
	 * @param color is in the format of the Cdk format strings.
	 * For more information see cdk_display (3).
	*/
	void setBackgroundColor(const char * color)
	{
		setCDKLabelBackgroundColor(_ptr.get(),color);
	}
	/**
	 * @brief Sets whether the widget will be drawn with a box around it.
	*/
	void setBox(bool box)
	{
		setCDKLabelBox(_ptr.get(),box);
	}
	/**
	 * @brief Sets the attribute of the box.
	*/
	void setBoxAttribute(chtype character)
	{
		setCDKLabelBoxAttribute(_ptr.get(),character);
	}
	/**
	 * @brief Sets the horizontal drawing character for the box to the given character.
	*/
	void setHorizontalChar(chtype character)
	{
		setCDKLabelHorizontalChar(_ptr.get(),character);
	}
	/**
	 * @brief Sets the lower left hand corner of the widget's box to the given character.
	*/
	void setLLChar(chtype character)
	{
		setCDKLabelLLChar(_ptr.get(),character);
	}
	/**
	 * @brief Sets the lower right hand corner of the widget's box to the given character.
	*/
	void setLRChar(chtype character)
	{
		setCDKLabelLRChar(_ptr.get(),character);
	}
	/**
	 * @brief This sets the contents of the label widget.
	*/
	void setMessage(StringList message)
	{
		auto v = transformStringList(message);
		setCDKLabelMessage(_ptr.get(),
						   v.data(),v.size());
	}
	/**
	 * @brief Sets the upper left hand corner of the widget's box to the given character.
	*/
	void setULChar(chtype character)
	{
		setCDKLabelULChar(_ptr.get(),character);
	}
	/**
	 * @brief Sets the upper right hand corner of the widget's box to the given character.
	*/
	void setURChar(chtype character)
	{
		setCDKLabelURChar(_ptr.get(),character);
	}
	/**
	 * @brief Sets the vertical drawing character for the box to the given character.
	*/
	void setVerticalChar(chtype character)
	{
		setCDKLabelVerticalChar(_ptr.get(),character);
	}
	/**
	 * @brief Waits for a user to press a key.
	 * @param key is the key to wait for.
	 * If no specific key is desired, use (char)0.
	*/
	char wait(char key)
	{
		return waitCDKLabel(_ptr.get(),key);
	}
	EObjectType type{vLABEL};

};

/** @brief Create and manage a curses button widget.
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
	 * @brief Activates the button widget and lets the user interact with the widget.
	 * @param actions If non-NULL, the characters in the array will be  injected into the widget.
	 * @return If the character entered into this widget is RETURN or TAB then this function will return a value from 0 to
				the number of buttons -1, representing the button selected.  It will also set the widget data  exitType  to
				vNORMAL.

			If the character entered into this widget was ESCAPE then the widget will return a value of -1 and the wid‐
			get data exitType will be set to vESCAPE_HIT.
	 */
	int activate(chtype * actions)
	{
		return activateCDKButton (_ptr.get(),actions);
	}
	/**
	 * @brief Draws the button widget on the screen.
	 * @param box whether the widget is drawn with a box.
	 */
	void draw (bool box)
	{
		drawCDKButton (_ptr.get(),box);
	}
	/**
	 * @brief Removes the widget from the screen.  This does NOT destroy the widget.
	 */
	void erase()
	{
		eraseCDKButton(_ptr.get());
	}
	/**
	 * @brief Returns true if the widget will be drawn with a box around it.
	 */
	bool getBox()
	{
		return getCDKButtonBox (_ptr.get());
	}
	/**
	 * @brief Returns the contents of the button widget.
	 */
	std::string_view getMessage()
	{
		auto msg = getCDKButtonMessage(_ptr.get());
		return {reinterpret_cast<char*>(msg)};
	}
	/**
	 * @brief Injects a single character into the widget.
	 * @param input the character to inject into the widget.
	 * @return
	 */
	int inject(chtype input)
	{
		return injectCDKButtonbox (_ptr.get(),input);
	}
	/**
	 * @brief set
	 * @param message
	 * @param box
	 */
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
	 * @brief Moves the given widget to the given position.
	 * @param p The new position of the widget.
	 * @param o the options
	 */
	void move(point p,move_options o)
	{
		moveCDKButton(_ptr.get(),
					  p.x,p.y,
					  o.relative,o.refresh);
	}
	/**
	 * @brief Allows  the user to move the widget around the screen via the cursor/keypad keys.
	 * @sa cdk_position (3) for key bindings.
	 */
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

/**
 * @brief Text entry widget
 */
class text_entry : public widget
{
	struct deleter
	{
		void operator()(CDKENTRY* p)
		{
			destroyCDKEntry(p);
		}
	};
	using entryptr = std::unique_ptr<CDKENTRY,deleter>;
	entryptr _ptr;
public:
	text_entry(screen& parent,point p, std::string_view title,std::string_view label ,
	           chtype fieldAttribute,
	           chtype fillerCharacter,
	           EDisplayType displayType,
	           int fieldWidth,
	           int minimumLength,
	           int maximumLength,
	           drawing_options o)
	{
		_ptr = entryptr(newCDKEntry(parent._ptr.get(),
		           p.x,p.y,
		           title.data(),
		           label.data(),
		           fieldAttribute,
		           fillerCharacter,
		           displayType,
		           fieldWidth,
		           minimumLength,
		           maximumLength,
		           o.box,
		           o.shadow));
		_vptr = _ptr.get();
	}
	char* activate(chtype *actions)
	{
		return activateCDKEntry (_ptr.get(),actions);
	}
	void clean()
	{
		cleanCDKEntry(_ptr.get());
	}
	void draw(bool box)
	{
		drawCDKEntry(_ptr.get(),box);
	}
	void erase()
	{
		eraseCDKEntry (_ptr.get());
	}
	bool getBox()
	{
		return getCDKEntryBox(_ptr.get());
	}
	chtype getFillerChar ()
	{
		return getCDKEntryFillerChar(_ptr.get());
	}
	chtype getHiddenChar()
	{
		return getCDKEntryHiddenChar(_ptr.get());
	}
	int getMax()
	{
		return getCDKEntryMax(_ptr.get());
	}
	int getMin()
	{
		return getCDKEntryMin(_ptr.get());
	}
	char* getValue()
	{
		return getCDKEntryValue (_ptr.get());
	}

	char* inject(chtype input)
	{
		return injectCDKEntry (_ptr.get(),input);
	}

	void move(point p,move_options o)
	{
		moveCDKEntry (_ptr.get(),
		                p.x,p.y,
		                o.relative,
		                o.refresh);
	}
	void position()
	{
		positionCDKEntry(_ptr.get());
	}

	void set(std::string_view value,
	        int minimumLength,
	        int maximumLength,
	        bool box)
	{
		setCDKEntry(_ptr.get(),
		            value.data(),
		            minimumLength,
		            maximumLength,
		            box);
	}
	void setBackgroundAttrib(chtype attribute)
	{
		setCDKEntryBackgroundAttrib(_ptr.get(),attribute);
	}
	void setBackgroundColor(const char * color)
	{
		setCDKEntryBackgroundColor(_ptr.get(),color);
	}
	void setBox(bool box)
	{
		setCDKEntryBox(_ptr.get(),box);
	}
	void setBoxAttribute (chtype character)
	{
		setCDKEntryBoxAttribute(_ptr.get(),character);
	}
	void setCB(ENTRYCB callBackFunction)
	{
		setCDKEntryCB (_ptr.get(),callBackFunction);
	}

	void setFillerChar(chtype character)
	{
		setCDKEntryFillerChar(_ptr.get(),character);
	}

	void setHiddenChar(chtype character)
	{
		setCDKEntryHiddenChar (_ptr.get(),character);
	}

	void setHighlight (chtype highlight,bool cursor)
	{
		setCDKEntryHighlight (_ptr.get(),highlight,cursor);
	}
	void setHorizontalChar(chtype character)
	{
		setCDKEntryHorizontalChar (_ptr.get(),character);
	}
	void setLLChar(chtype character)
	{
		setCDKEntryLLChar (_ptr.get(),character);
	}

	void setLRChar(chtype character)
	{
		setCDKEntryLRChar(_ptr.get(),character);
	}
	void setMax(int maximum)
	{
		setCDKEntryMax(_ptr.get(),maximum);
	}
	void setMin(int minimum)
	{
		setCDKEntryMin(_ptr.get(),minimum);
	}

	void setPostProcess(PROCESSFN callback,void * data)
	{
		setCDKEntryPostProcess (_ptr.get(),callback,data);
	}
	void setPreProcess(PROCESSFN callback,void * data)
	{
		setCDKEntryPreProcess (_ptr.get(),callback,data);
	}
	void setULChar(chtype character)
	{
		setCDKEntryULChar(_ptr.get(),character);
	}
	void setURChar(chtype character)
	{
		setCDKEntryURChar(_ptr.get(),character);
	}
	void setValue(std::string_view value)
	{
		setCDKEntryValue(_ptr.get(),value.data());
	}
	void setVerticalChar (chtype character)
	{
		setCDKEntryVerticalChar(_ptr.get(),character);
	}

	EObjectType type{vENTRY};
};

/**
 * @brief Sorted list widget.
 * alpha_list allows a user to select from a list of alphabetically sorted words.  The
	   user can use the arrow keys to traverse through the list or type in the beginning  of  the
	   word  and  the list will automatically adjust itself in the correct place in the scrolling
	   list.  This widget, like the file selector widget, is a compound widget of both the  entry
	   field widget and the scrolling list widget.
 */
class alpha_list : public widget
{
	struct deleter
	{
		void operator()(CDKALPHALIST* p)
		{
			destroyCDKAlphalist(p);
		}
	};
	using alphalistptr = std::unique_ptr<CDKALPHALIST,deleter>;
	alphalistptr _ptr;
public:
	alpha_list() = default;


	alpha_list(screen& parent, point p, widget_size size,
	        std::string_view title,
	        std::string_view  label,
	        StringList list,
	        chtype fillerCharacter,
	        chtype highlight,
	        drawing_options o)
	{
		_ptr = alphalistptr(newCDKAlphalist (
		           parent._ptr.get(),
		           p.x,
		           p.y,
		           size.height,
		           size.width,
		           title.data(),
		           label.data(),
		           transformStringList( list).data(),
		           list.size(),
		           fillerCharacter,
		           highlight,
		           o.box,
		           o.shadow));
		_vptr = _ptr.get();
	}
	std::string activate(chtype* actions)
	{
		return std::string(activateCDKAlphalist(_ptr.get(),actions));
	}
	void draw(bool box)
	{
		drawCDKAlphalist(_ptr.get(),box);
	}
	void erase()
	{
		eraseCDKAlphalist(_ptr.get());
	}
	bool getBox()
	{
		return getCDKAlphalistBox(_ptr.get());
	}
	StringList getContents()
	{
		int items;
		char** contents = getCDKAlphalistContents (_ptr.get(),&items);
		return transformCharPtrPtr(contents,items);
	}
	int getCurrentItem()
	{
		return getCDKAlphalistCurrentItem(_ptr.get());
	}
	chtype getFillerChar()
	{
		return getCDKAlphalistFillerChar (_ptr.get());
	}
	chtype getHighlight()
	{
		return getCDKAlphalistHighlight(_ptr.get());
	}
	char* inject(chtype input)
	{
		return injectCDKAlphalist(_ptr.get(),input);
	}
	void move(point p,move_options o)
	{
		moveCDKAlphalist(_ptr.get(), p.x, p.y, o.relative, o.refresh);
	}
	void position()
	{
		positionCDKAlphalist(_ptr.get());
	}
	void set(StringList list,chtype fillerCharacter,chtype highlight,bool box)
	{
		setCDKAlphalist(_ptr.get(),
		        transformStringList(list).data(),
		        list.size(),
		        fillerCharacter,
		        highlight,
		        box);
	}
	void setBackgroundAttrib(chtype attribute)
	{
		setCDKAlphalistBackgroundAttrib (_ptr.get(),attribute);
	}
	void setBackgroundColor (const char* color)
	{
		setCDKAlphalistBackgroundColor(_ptr.get(),color);
	}
	void setBox(bool box)
	{
		setCDKAlphalistBox(_ptr.get(),box);
	}
	void setBoxAttribute(chtype character)
	{
		setCDKAlphalistBoxAttribute(_ptr.get(),character);
	}
	void setContents (StringList c)
	{
		setCDKAlphalistContents(_ptr.get(),transformStringList(c).data(),c.size());
	}
	void setCurrentItem(int item)
	{
		setCDKAlphalistCurrentItem (_ptr.get(),item);
	}
	void setFillerChar(chtype fillerCharacter)
	{
		setCDKAlphalistFillerChar(_ptr.get(),fillerCharacter);
	}
	void setHighlight(chtype highlight)
	{
		setCDKAlphalistHighlight (_ptr.get(),highlight);
	}
	void setHorizontalChar(chtype character)
	{
		setCDKAlphalistHorizontalChar (_ptr.get(),character);
	}
	void setLLChar(chtype character)
	{
		setCDKAlphalistLLChar(_ptr.get(),character);
	}
	void setLRChar(chtype character)
	{
		setCDKAlphalistLRChar(_ptr.get(),character);
	}
	void setPostProcess (PROCESSFN callback,void * data)
	{
		setCDKAlphalistPostProcess (_ptr.get(),callback, data);
	}
	void setPreProcess(PROCESSFN callback,void * data)
	{
		setCDKAlphalistPreProcess(_ptr.get(),callback,data);
	}
	void setULChar(chtype character)
	{
		setCDKAlphalistULChar (_ptr.get(),character);
	}
	void setURChar(chtype character)
	{
		setCDKAlphalistURChar (_ptr.get(),character);
	}
	void setVerticalChar(chtype character)
	{
		setCDKAlphalistVerticalChar(_ptr.get(),character);
	}


	EObjectType type{vALPHALIST};
};

struct date {
	int day;
	int month;
	int year;

};
struct date_attributes
{
	chtype day;
	chtype month;
	chtype year;
};

/**
 * @brief The calendar class curses calendar widget.
 */
class calendar : public widget
{
	struct deleter
	{
		void operator()(CDKCALENDAR* p)
		{
			destroyCDKCalendar(p);
		}
	};
	using calendarptr = std::unique_ptr<CDKCALENDAR,deleter>;
	calendarptr _ptr;
public:
	/**Empty constructor
	  */
	calendar() = default;
	/**
	 * @brief creates a calendar widget
	 * @param parent the screen you wish this widget to be placed in.
	 * @param p widget's position
	 * @param title the string which will be displayed at the top of the wid‐
				 get.  The title can be more than one  line;  just  provide  a
				 carriage return character at the line break.
	 * @param d initial date of the calendar.
	 * @param attr the attributes of the year, month, and day respectively.
	 * @param highlight sets the highlight of the currently selected day.
	 * @param o drawing options
	 * @sa drawing_options point
	 */
	calendar(screen parent,
	         point p,
	         std::string_view title,
	         date d,
	         date_attributes attr,
	         chtype highlight,
	         drawing_options o)
	{
		_ptr = calendarptr(newCDKCalendar (parent._ptr.get(),
		                                p.x,p.y,
		                                title.data(),
		                                d.day,
		                                d.month,
		                                d.year,
		                                attr.day,
		                                attr.month,
		                                attr.year,
		                                highlight,
		                                o.box,
		                                o.shadow));
		_vptr = _ptr.get();

	}
	/**
	 * @brief  activates the calendar widget and lets the user interact with  the
			widget.
	 * @param actions If the actions parameter is passed with a non-NULL value,  the
				characters in the array will be injected into the widget.
				To  activate  the  widget interactively pass in a NULL pointer
				for actions.
	 * @return If the character entered into this widget is RETURN then  this
				function will return a type of time_t.
				If the character entered into this widget was ESCAPE or  TAB  then
				this  function  will  return  a value of (time_t)-1 and the widget
				data exitType will be set to vESCAPE_HIT.
	 */
	time_t activate(chtype *actions)
	{
		return activateCDKCalendar(_ptr.get(),actions);
	}
	/**
	 * @brief draws the label widget on the screen.
	 * @param box If the box parameter is true, the widget is drawn with a box.
	 */
	void draw(bool box)
	{
		drawCDKCalendar (_ptr.get(),box);
	}
	/**
	 * @brief removes  the  widget  from  the screen.  This does NOT destroy the
			widget.
	 */
	void erase()
	{
		eraseCDKCalendar(_ptr.get());
	}
	/**
	 * @return whether the widget will be drawn with a box around it.
	 */
	bool getBox()
	{
		return getCDKCalendarBox(_ptr.get());
	}
	/**
	 * @return the current date the calendar is displaying.
	 */
	date getDate()
	{
		date d;
		getCDKCalendarDate (_ptr.get(),
		                    &d.day,
		                    &d.month,
		                    &d.year);
		return d;
	}
	/**
	 * @return the attribute of the day attribute of the calendar.
	 */
	chtype getDayAttribute()
	{
		return getCDKCalendarDayAttribute(_ptr.get());
	}
	/**
	 * @return the attribute of the highlight bar of the scrolling list portion of the widget.
	 */
	chtype getHighlight()
	{
		return getCDKCalendarHighlight(_ptr.get());
	}
	/**
	 * @param d the date passed to setMarker
	 * @return the marker set on the calendar by setMarker.
	 * @sa setMarker
	 */
	chtype getMarker(date d)
	{
		return getCDKCalendarMarker(_ptr.get(),
		                            d.day,
		                            d.month,
		                            d.year);
	}
	/**
	 * @return the attribute of the month attribute of the calendar.
	 */
	chtype getMonthAttribute()
	{
		return getCDKCalendarMonthAttribute(_ptr.get());
	}
	/**
	 * @return the attribute of the year attribute of the calendar.
	 */
	chtype getYearAttribute()
	{
		return getCDKCalendarYearAttribute(_ptr.get());
	}
	/**
	 * @brief injects a single character into the widget.
	 * @param input the character to inject into the widget.
	 * @return The return value and side-effect (setting the widget data exitType) depend upon the injected  char‐
			acter:

			- RETURN or TAB
				   the  function  returns a value of type time_t (see localtime or ctime for more information).
				   The widget data exitType is set to vNORMAL.

			- ESCAPE the function returns (time_t)-1.  The widget data exitType is set to vESCAPE_HIT.

			- Otherwise
				   unless modified by preprocessing, postprocessing  or  key  bindings,  the  function  returns
				   (time_t)-1.  The widget data exitType is set to vEARLY_EXIT.
	 */
	time_t inject(chtype input)
	{
		return injectCDKCalendar(_ptr.get(),input);
	}
	/**
	 * @brief moves the given widget to the given position.
	 * @param p the new position of the widget.
	 * @param o the move options
	 * @sa move_options
	 */
	void move(point p,move_options o)
	{
		moveCDKCalendar(_ptr.get(),
		                p.x,p.y,
		                o.relative,o.refresh);
	}
	/**
	 * @brief allows  the user to move the widget around the screen via the cursor/keypad keys.
	 */
	void position()
	{
		positionCDKCalendar(_ptr.get());
	}
	/**
	 * @brief removes a marker from the calendar created with setMarker
	 * @param d the date where the marker is.
	 */
	void removeMarker(date d)
	{
		removeCDKCalendarMarker(_ptr.get(),
		                   d.day,
		                   d.month,
		                   d.year);
	}
	/**
	 * @brief lets the programmer modify elements of an existing calendar widget.
	 * @param d
	 * @param attr
	 * @param highlight
	 * @param box
	 */
	void set(date d,date_attributes attr,chtype highlight,bool box)
	{
		setCDKCalendar (_ptr.get(),
		                   d.day,
		                   d.month,
		                   d.year,
		                   attr.day,
		                   attr.month,
		                   attr.year,
		                   highlight,
		                   box);
	}
	void setBackgroundAttrib (chtype attribute)
	{
		setCDKCalendarBackgroundAttrib (_ptr.get(),attribute);
	}
	void setBackgroundColor(std::string_view color)
	{
		setCDKCalendarBackgroundColor(_ptr.get(),color.data());
	}
	void setBox(bool box)
	{
		setCDKCalendarBox (_ptr.get(),box);
	}
	void setBoxAttribute(chtype ch)
	{
		setCDKCalendarBoxAttribute (_ptr.get(),ch);
	}
	void setDate(date d)
	{
		setCDKCalendarDate (_ptr.get(),
		                   d.day,
		                   d.month,
		                   d.year);
	}
	void setDayAttribute (chtype attribute)
	{
		setCDKCalendarDayAttribute (_ptr.get(),attribute);
	}
	void setDaysNames(std::string_view days)
	{
		setCDKCalendarDaysNames (_ptr.get(),days.data());
	}
	void setHighlight (chtype attribute)
	{
		setCDKCalendarHighlight(_ptr.get(),attribute);
	}
	void setHorizontalChar(chtype ch)
	{
		setCDKCalendarHorizontalChar(_ptr.get(),ch);
	}
	void setLLChar(chtype ch)
	{
		setCDKCalendarLLChar(_ptr.get(),ch);
	}
	void setLRChar (chtype ch)
	{
		setCDKCalendarLRChar(_ptr.get(),ch);
	}
	/**
	 * @brief allows the user to set a marker which will be displayed when the month is drawn.
	 * @param d the date to display the marker
	 * @param marker the attribute to use when drawing the marker.  If more than one  marker  is
			set on a single day, then the day will blink with the original marker attribute.
	 */
	void setMarker (date d,chtype marker)
	{
		setCDKCalendarMarker(_ptr.get(),
		                   d.day,
		                   d.month,
		                   d.year,
		                   marker);
	}
	void setMonthAttribute(chtype attribute)
	{
		setCDKCalendarMonthAttribute(_ptr.get(),attribute);
	}
	void setMonthsNames(StringList months)
	{
		setCDKCalendarMonthsNames(_ptr.get(),transformStringList( months).data());
	}
	void setPostProcess(PROCESSFN callback,void * data)
	{
		setCDKCalendarPostProcess(_ptr.get(),callback,data);
	}
	void setPreProcess(PROCESSFN callback,void * data)
	{
		setCDKCalendarPreProcess(_ptr.get(),callback,data);
	}
	void setULChar(chtype ch)
	{
		setCDKCalendarULChar(_ptr.get(),ch);
	}
	void setURChar(chtype ch)
	{
		setCDKCalendarURChar (_ptr.get(),ch);
	}
	void setVerticalChar(chtype ch)
	{
		setCDKCalendarVerticalChar(_ptr.get(),ch);
	}

	void setYearAttribute(chtype attribute)
	{
		setCDKCalendarYearAttribute (_ptr.get(),attribute);
	}

	EObjectType type{vCALENDAR};
};

}//namespace cdk
























