#include "LinearLightList.h"

namespace render
{

void LinearLightList::calculateIntersectingLights() const
{
    // Get the renderer to tell us whether anything actually needs updating
    _testDirtyFunc();

    if (m_dirty)
    {
        m_dirty = false;

        _activeLights.clear();
        _litObject.clearLights();

        // Determine which lights intersect object
        for (RendererLight* light : _allLights)
        {
            if (_litObject.intersectsLight(*light))
            {
                _activeLights.push_back(light);
                _litObject.insertLight(*light);
            }
        }
    }
}

void LinearLightList::forEachLight(const RendererLightCallback& callback) const
{
    calculateIntersectingLights();

    for (RendererLight* light : _activeLights) 
    { 
        callback(*light);
    }
}

void LinearLightList::setDirty()
{
    m_dirty = true;
}

}
