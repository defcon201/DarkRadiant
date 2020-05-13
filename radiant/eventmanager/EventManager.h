#pragma once

#include "ieventmanager.h"
#include <wx/event.h>
#include <sigc++/connection.h>

#include <map>
#include <list>

#include "xmlutil/Node.h"
#include "Accelerator.h"

#include "GlobalKeyEventFilter.h"

namespace ui
{

class EventManager :
	public IEventManager,
	public wxEvtHandler
{
private:
	// Each command has a name, this is the map where the name->command association is stored
	typedef std::map<const std::string, IEventPtr> EventMap;

	struct ItemConnection
	{
		IMenuElementPtr item;
		sigc::connection itemActivatedConn;
	};

	std::multimap<std::string, ItemConnection> _menuItems;

	// The command-to-accelerator map containing all registered shortcuts
	std::map<std::string, Accelerator::Ptr> _accelerators;

	// The map of all registered events
	EventMap _events;

	IEventPtr _emptyEvent;
	Accelerator _emptyAccelerator;

    GlobalKeyEventFilterPtr _shortcutFilter;

public:
	// RegisterableModule implementation
	const std::string& getName() const override;
	const StringSet& getDependencies() const override;
	void initialiseModule(const ApplicationContext& ctx) override;
	void shutdownModule() override;

	// Constructor
	EventManager();

	IEventPtr findEvent(const std::string& name) override;
	IEventPtr findEvent(wxKeyEvent& ev) override;

	std::string getEventName(const IEventPtr& event) override;

	std::string getAcceleratorStr(const IEventPtr& event, bool forMenu) override;

	void resetAcceleratorBindings() override;

	// Checks if the eventName is already registered and writes to rMessage, if so
	bool alreadyRegistered(const std::string& eventName);

	// Add a command and specify the statement to execute when triggered
	IEventPtr addCommand(const std::string& name, const std::string& statement, bool reactOnKeyUp) override;

	IEventPtr addKeyEvent(const std::string& name, const KeyStateChangeCallback& keyStateChangeCallback) override;
	IEventPtr addWidgetToggle(const std::string& name) override;
	IEventPtr addRegistryToggle(const std::string& name, const std::string& registryKey) override;
	IEventPtr addToggle(const std::string& name, const ToggleCallback& onToggled) override;

	void setToggled(const std::string& name, const bool toggled) override;

	void registerMenuItem(const std::string& eventName, const ui::IMenuElementPtr& item) override;
	void unregisterMenuItem(const std::string& eventName, const ui::IMenuElementPtr& item) override;

	// Connects the given accelerator to the given command (identified by the string)
	void connectAccelerator(wxKeyEvent& keyEvent, const std::string& command) override;
	void disconnectAccelerator(const std::string& command) override;

	void disableEvent(const std::string& eventName) override;
	void enableEvent(const std::string& eventName) override;

	void renameEvent(const std::string& oldEventName, const std::string& newEventName) override;
	void removeEvent(const std::string& eventName) override;

	void disconnectToolbar(wxToolBar* toolbar) override;

	// Loads the default shortcuts from the registry
	void loadAccelerators() override;

	void foreachEvent(IEventVisitor& eventVisitor) override;

	// Tries to locate an accelerator, that is connected to the given command
	Accelerator& findAccelerator(wxKeyEvent& ev);
	bool handleKeyEvent(wxKeyEvent& keyEvent);

	std::string getEventStr(wxKeyEvent& ev) override;

private:
	void saveEventListToRegistry();

	Accelerator& connectAccelerator(int keyCode, unsigned int modifierFlags, const std::string& command);

	Accelerator& findAccelerator(const IEventPtr& event);
	Accelerator& findAccelerator(const std::string& commandName);
	Accelerator& findAccelerator(const std::string& key, const std::string& modifierStr);

	bool duplicateAccelerator(const std::string& key, const std::string& modifiers, const IEventPtr& event);

	// Returns the pointer to the accelerator for the given event, but convert the key to uppercase before passing it
	Accelerator& findAccelerator(unsigned int keyVal, const unsigned int modifierFlags);

	void loadAcceleratorFromList(const xml::NodeList& shortcutList);

	void setMenuItemAccelerator(const std::string& command, const std::string& acceleratorStr);

	bool isModifier(wxKeyEvent& ev);
};

}
