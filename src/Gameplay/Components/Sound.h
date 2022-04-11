#pragma once
#include "fmod.hpp"
#include <string>
#include <unordered_map>
#include "fmod_studio.hpp"

class Sound
{
public:
	static int errorcheck(FMOD_RESULT Result);
	void init();
	void update();
	void shutdown();
	void loadsound(const std::string& soundName, const std::string& filename, bool b3d, bool soundloop = false, bool bstream = false);
	void unloadsound(const std::string& soundName);
	void playsound(const std::string& soundName);
	
private:
	FMOD::Studio::System* Playback;
	std::unordered_map<std::string, FMOD::Sound*>Sounds;
	FMOD::System* firstPlayback;
};



