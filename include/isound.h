#pragma once

#include "imodule.h"
#include "ModResource.h"

#include <vector>
#include <functional>
#include <sigc++/signal.h>

// A list of sound files associated to a shader
typedef std::vector<std::string> SoundFileList;

const float METERS_PER_UNIT = 0.0254f;
const float UNITS_PER_METER = 1/METERS_PER_UNIT;

// The min and max radii of a sound shader
class SoundRadii {
    float minRad, maxRad;
    public:
    //set sound radii either in metres or in inch on initialization might cause a conversion
    SoundRadii (float min = 0, float max = 0, bool inMetres = false) {
        if (inMetres) {
            minRad = min * UNITS_PER_METER;
            maxRad = max * UNITS_PER_METER;
        }
        else {
            minRad = min;
            maxRad = max;
        }
    }

    // set the sound radii in metres or in inch, might cause a conversion
    void setMin(float min, bool inMetres = false) {
        if (inMetres) {
            minRad = min * UNITS_PER_METER;
        }
        else {
            minRad = min;
        }
    }

    void setMax (float max, bool inMetres = false) {
        if (inMetres) {
            maxRad = max * UNITS_PER_METER;
        }
        else {
            maxRad = max;
        }
    }

    float getMin(bool inMetres = false) const {
        return (inMetres) ? minRad * METERS_PER_UNIT : minRad;
    }

    float getMax(bool inMetres = false) const {
        return (inMetres) ? maxRad * METERS_PER_UNIT : maxRad;
    }
};

/// Representation of a single sound or sound shader.
class ISoundShader :
    public ModResource
{
public:
    virtual ~ISoundShader() {}

    /// Get the name of the shader
    virtual std::string getName() const = 0;

    /// Get the min and max radii of the shader
    virtual SoundRadii getRadii() const = 0;

    /// Get the list of sound files associated to this shader
    virtual SoundFileList getSoundFileList() const = 0;

	// angua: get the display folder for sorting the sounds in the sound chooser window
	virtual const std::string& getDisplayFolder() const = 0;

    // Returns the mod-relative path to the file this shader is defined in
    virtual std::string getShaderFilePath() const = 0;

    // Returns the raw sound shader definition text
    virtual std::string getDefinition() const = 0;

};
typedef std::shared_ptr<ISoundShader> ISoundShaderPtr;

const char* const MODULE_SOUNDMANAGER("SoundManager");

/// Sound manager interface.
class ISoundManager :
    public RegisterableModule
{
public:
    /// Invoke a function for each sound shader
    virtual void forEachShader(std::function<void(const ISoundShader&)>) = 0;

    /** greebo: Tries to lookup the SoundShader with the given name,
     *          returns a soundshader with an empty name, if the lookup failed.
     */
    virtual ISoundShaderPtr getSoundShader(const std::string& shaderName) = 0;

    /** 
	 * greebo: Plays the given sound file (defined by its VFS path).
     *
     * @returns: TRUE, if the sound file was found at the given VFS path,
     *           FALSE otherwise
     */
    virtual bool playSound(const std::string& fileName) = 0;

	/** 
	 * greebo: Plays the given sound file (defined by its VFS path).
	 * Will loop the sound if the given flag is set to TRUE.
	 *
	 * @returns: TRUE, if the sound file was found at the given VFS path,
	 *           FALSE otherwise
	 */
	virtual bool playSound(const std::string& fileName, bool loopSound) = 0;

    /** greebo: Stops the currently played sound.
     */
    virtual void stopSound() = 0;

    // Returns the duration of the given sound file in seconds
    // Will throw a std::out_of_range exception if the path cannot be resolved
    virtual float getSoundFileDuration(const std::string& vfsPath) = 0;

    // Reloads all sound shader definitions from the VFS
    virtual void reloadSounds() = 0;

    // Fired after the sound shaders have been (re-)parsed from disk
    virtual sigc::signal<void>& signal_soundShadersReloaded() = 0;
};

// Accessor method
inline ISoundManager& GlobalSoundManager() 
{
    static module::InstanceReference<ISoundManager> _reference(MODULE_SOUNDMANAGER);
    return _reference;
}
