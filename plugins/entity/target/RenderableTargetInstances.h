#ifndef _ENTITY_RENDERABLETARGETINSTANCES_H_
#define _ENTITY_RENDERABLETARGETINSTANCES_H_

#include "Target.h"
#include <set>
#include "renderable.h"

namespace entity {

// Forward declaration
class TargetableInstance;

/**
 * greebo: This is a "container" for all TargetableInstances. These register
 *         themselves at construction time and will get invoked during
 *         the frontend render pass.
 *
 * This object is also a Renderable which is always attached to the GlobalShaderCache()
 * during the entire module lifetime.
 */
class RenderableTargetInstances : 
	public Renderable
{
	typedef std::set<TargetableInstance*> TargetableInstances;
	TargetableInstances _instances;

public:
	// Add/Remove a TargetableInstance to this set
	void attach(TargetableInstance& instance);
	void detach(TargetableInstance& instance);

	// Renderable implementation
	void renderSolid(Renderer& renderer, const VolumeTest& volume) const;
	void renderWireframe(Renderer& renderer, const VolumeTest& volume) const;

	// Accessor method to the singleton instance
	static RenderableTargetInstances& Instance();
};

} // namespace entity

#endif /* _ENTITY_RENDERABLETARGETINSTANCES_H_ */
