#pragma once

#include <SDL3/SDL.h>
#include <memory>

enum class SoundCue { Select, Launch, Complete, Failure };

class GameAudio {
public:
    bool initialize();
    void setVolume(int percent);
    void play(SoundCue cue);

private:
    std::unique_ptr<SDL_AudioStream, decltype(&SDL_DestroyAudioStream)> stream{
        nullptr, SDL_DestroyAudioStream};
};
