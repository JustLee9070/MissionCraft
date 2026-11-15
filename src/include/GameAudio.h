
#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <cstddef>

enum class SoundCue { Select, Launch, Complete, Failure };

class GameAudio {
public:
    bool initialize();
    void setVolume(int percent);
    void setMusicVolume(int percent);
    void play(SoundCue cue);
    bool playMusic(const char* path);
    void stopMusic();

private:
    std::unique_ptr<SDL_AudioStream, decltype(&SDL_DestroyAudioStream)> stream{
        nullptr, SDL_DestroyAudioStream};

    std::unique_ptr<SDL_AudioStream, decltype(&SDL_DestroyAudioStream)> musicStream{
        nullptr, SDL_DestroyAudioStream};

    std::vector<Uint8> musicSamples;
    size_t musicPosition = 0;

    int sfxVolume = 100;
    int musicVolume = 25;

    static void SDLCALL feedMusic(void* userdata, SDL_AudioStream* stream,
                                  int additionalAmount, int totalAmount);
};
