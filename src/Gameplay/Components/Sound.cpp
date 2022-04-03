#include "Sound.h"
#include "fmod_errors.h"
#include <iostream>

int Sound::errorcheck(FMOD_RESULT Result) 
{
	if (Result != FMOD_OK) {
		std::cout << "Fmod Error: " << FMOD_ErrorString(Result);

	#ifdef DEBUG
		__debugbreak();
	#endif // DEBUG
		return 1;
	}
	return 0;
}


void Sound::init()
{
	errorcheck(FMOD::System_Create(&Playback));
	errorcheck(Playback->init(32, FMOD_INIT_NORMAL, nullptr));

}

void Sound::update()
{
	errorcheck(Playback->update());
}

void Sound::shutdown()
{
	errorcheck(Playback->close());
	errorcheck(Playback->release());
}

void Sound::loadsound(const std::string& soundName, const std::string& filename, bool b3d, bool soundloop, bool bstream)
{
	auto foundElement = Sounds.find(soundName);
	if (foundElement != Sounds.end()) 
	{
		return;
	}

	FMOD_MODE mode = FMOD_DEFAULT;
	mode |= (b3d) ? FMOD_3D : FMOD_2D;
	mode |= (soundloop) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
	mode |= (bstream) ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

	FMOD::Sound* LoadedSound;
	errorcheck(Playback->createSound(filename.c_str(), mode, nullptr, &LoadedSound));

	if (LoadedSound != nullptr) {
		Sounds[soundName] = LoadedSound;
	}
}

void Sound::unloadsound(const std::string& soundName)
{
	auto foundElement = Sounds.find(soundName);
	if (foundElement != Sounds.end()) 
	{
		errorcheck(foundElement->second->release());
		Sounds.erase(foundElement);
	}
}

void Sound::playsound(const std::string& soundName)
{
	Playback->playSound(Sounds[soundName], nullptr, false, nullptr);
}
