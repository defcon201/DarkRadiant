#ifndef FLOATINGORTHOVIEW_H_
#define FLOATINGORTHOVIEW_H_

#include "XYWnd.h"

#include <iostream>

#include "gtkutil/window/PersistentTransientWindow.h"
#include "gtkutil/FramedWidget.h"

/**
 * A floating version of the XYWnd. These are created by the XYWndManager and 
 * notify the manager when they are destroyed.
 */
class FloatingOrthoView
: public gtkutil::PersistentTransientWindow,
  public XYWnd
{
public:
	/**
	 * Construct a floating XY window with the given numeric ID (assigned by
	 * the XYWndManager).
	 * 
	 * @param id
	 * Unique ID assigned to this window.
	 * 
	 * @param title
	 * The displayed title for this window (e.g. "XY Front").
	 * 
	 * @param parent
	 * The parent window for which this should be a transient (normally the
	 * mainframe).
	 */
	FloatingOrthoView(int id, const std::string& title, const Glib::RefPtr<Gtk::Window>& parent)
	: gtkutil::PersistentTransientWindow(title, parent, false),
	  XYWnd(id)
	{
		// Get the GL widget from XYWnd parent, and embed it in a frame in the
		// floating window
		Gtk::Frame* framedWidget = Gtk::manage(new gtkutil::FramedWidget(*XYWnd::getWidget()));
		add(*framedWidget);
		
		// Set the parent window for XYWnd
		XYWnd::setParent(getRefPtr());
		
		set_type_hint(Gdk::WINDOW_TYPE_HINT_NORMAL);

		signal_focus_in_event().connect(sigc::mem_fun(*this, &FloatingOrthoView::onFocus));
	}

	/** Overrides the setViewType method of the XYWnd base class.
	 *  Extends the functionality by setting the window title.
	 */
	virtual void setViewType(EViewType viewType)
	{
		// Invoke the base class method first
		XYWnd::setViewType(viewType);
		
		// Get the title string and write it to the window
		set_title(getViewTypeTitle(viewType));
	}

protected:
	
	// Post-destroy callback, initiate destruction of this XYWnd
	virtual void _postDestroy()
	{
		// Tell the XYWndManager to release the shared_ptr of this instance.
		// Otherwise our destructor will never be called.
		GlobalXYWnd().destroyXYWnd(_id);
	}

private:
	bool onFocus(GdkEventFocus* ev)
	{
		// Let the global XYWndManager know about the focus change
		GlobalXYWnd().setActiveXY(_id);

		return false;
	}
};
typedef boost::shared_ptr<FloatingOrthoView> FloatingOrthoViewPtr;

#endif /*FLOATINGORTHOVIEW_H_*/
