#include "PropertyEditorFactory.h"

#include "isound.h"
#include "iuimanager.h"
#include "itextstream.h"
#include "Vector3PropertyEditor.h"
#include "BooleanPropertyEditor.h"
#include "EntityPropertyEditor.h"
#include "ColourPropertyEditor.h"
#include "TexturePropertyEditor.h"
#include "SkinPropertyEditor.h"
#include "SoundPropertyEditor.h"
#include "FloatPropertyEditor.h"
#include "ModelPropertyEditor.h"
#include "ClassnamePropertyEditor.h"
#include "AnglePropertyEditor.h"

#include <regex>

#include <wx/artprov.h>

namespace ui
{

// Initialisation
PropertyEditorFactory::PropertyEditorMap PropertyEditorFactory::_peMap;
PropertyEditorFactory::PropertyEditorMap PropertyEditorFactory::_customEditors;

// Register the classes
void PropertyEditorFactory::registerClasses()
{
    _peMap["vector3"] = PropertyEditorPtr(new Vector3PropertyEditor());
    _peMap["bool"] = PropertyEditorPtr(new BooleanPropertyEditor());
    _peMap["entity"] = PropertyEditorPtr(new EntityPropertyEditor());
	_peMap["colour"] = PropertyEditorPtr(new ColourPropertyEditor());
	_peMap["color"] = PropertyEditorPtr(new ColourPropertyEditor());
	_peMap["texture"] = PropertyEditorPtr(new TexturePropertyEditor());
	_peMap["mat"] = PropertyEditorPtr(new TexturePropertyEditor());
	_peMap["skin"] = PropertyEditorPtr(new SkinPropertyEditor());

	if (module::GlobalModuleRegistry().moduleExists(MODULE_SOUNDMANAGER))
	{
		_peMap["sound"] = PropertyEditorPtr(new SoundPropertyEditor());
	}

	_peMap["float"] = PropertyEditorPtr(new FloatPropertyEditor());
	_peMap["model"] = PropertyEditorPtr(new ModelPropertyEditor());
	_peMap["classname"] = PropertyEditorPtr(new ClassnamePropertyEditor());
    _peMap["angle"] = PropertyEditorPtr(new AnglePropertyEditor());
}

void PropertyEditorFactory::registerPropertyEditor(const std::string& key, const IPropertyEditorPtr& editor)
{
	std::pair<PropertyEditorMap::iterator, bool> result = _customEditors.insert(
		PropertyEditorMap::value_type(key, editor)
	);

	if (!result.second)
	{
		rWarning() << "Could not register property editor for key " << key <<
			", is already associated." << std::endl;;
	}
}

void PropertyEditorFactory::unregisterPropertyEditor(const std::string& key)
{
	PropertyEditorMap::iterator found = _customEditors.find(key);

	if (found != _customEditors.end())
	{
		_customEditors.erase(found);
	}
	else
	{
		rWarning() << "Cannot unregister property editor for key " << key << std::endl;
	}
}

// Create a PropertyEditor from the given name.
IPropertyEditorPtr PropertyEditorFactory::create(wxWindow* parent, const std::string& className,
												Entity* entity,
												const std::string& key,
												const std::string& options)
{
    // Register the PropertyEditors if the map is empty
    if (_peMap.empty()) {
        registerClasses();
    }

	// greebo: First, search the custom editors for a match
	for (PropertyEditorMap::const_iterator i = _customEditors.begin();
		 i != _customEditors.end(); ++i)
	{
		if (i->first.empty()) continue; // skip empty keys

		// Try to match the entity key against the regex (i->first)
		std::regex expr(i->first);
		std::smatch matches;

		if (!std::regex_match(key, matches, expr)) continue;

		// We have a match
		return i->second->createNew(parent, entity, key, options);
	}

	// No custom editor found, search for the named property editor type
	PropertyEditorMap::iterator iter(_peMap.find(className));

	// If the type is not found, return NULL otherwise create a new instance of
	// the associated derived type.
	if (iter == _peMap.end()) {
		return PropertyEditorPtr();
	} else {
		return iter->second->createNew(parent, entity, key, options);
	}
}

IPropertyEditorPtr PropertyEditorFactory::getRegisteredPropertyEditor(const std::string& key)
{
	// Register the PropertyEditors if the map is empty
    if (_peMap.empty()) {
        registerClasses();
    }

	// greebo: First, search the custom editors for a match
	for (PropertyEditorMap::const_iterator i = _customEditors.begin();
		 i != _customEditors.end(); ++i)
	{
		if (i->first.empty()) continue; // skip empty keys

		// Try to match the entity key against the regex (i->first)
		std::regex expr(i->first);
		std::smatch matches;

		if (!std::regex_match(key, matches, expr)) continue;

		// We have a match
		return i->second;
	}

	// No custom editor found, search for the named property editor type
	PropertyEditorMap::iterator iter(_peMap.find(key));

	// If the type is not found, return NULL otherwise create a new instance of
	// the associated derived type.
	return (iter != _peMap.end()) ? iter->second : PropertyEditorPtr();
}

// Return a GdkPixbuf containing the icon for the given property type

wxBitmap PropertyEditorFactory::getBitmapFor(const std::string& type)
{
	// Sanity check
	if (type.empty()) return wxNullBitmap;

	std::string iconName = "icon_" + type + ".png";

	wxBitmap candidate = wxArtProvider::GetBitmap(GlobalUIManager().ArtIdPrefix() + iconName);

	if (!candidate.IsOk())
	{
		candidate = wxArtProvider::GetBitmap(GlobalUIManager().ArtIdPrefix() + "empty.png");
	}

	return candidate;
}

}
