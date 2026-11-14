#include "GameAudio.h"

#include <limits>

namespace {
constexpr SDL_AudioSpec playback{SDL_AUDIO_S16, 1, 24000};

const char* cuePath(SoundCue cue)
{
    switch (cue) {
    case SoundCue::Select: return "assets/audio/sfx/select.wav";
    case SoundCue::Launch: return "assets/audio/sfx/launch.wav";
    case SoundCue::Complete: return "assets/audio/sfx/complete.wav";
    case SoundCue::Failure: return "assets/audio/sfx/failure.wav";
    }
    return nullptr;
}
}

bool GameAudio::initialize()
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        return false;
    }
    stream.reset(SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &playback,
                                          nullptr, nullptr));
    if (!stream || !SDL_ResumeAudioStreamDevice(stream.get())) {
        stream.reset();
        return false;
    }
    return true;
}

void GameAudio::setVolume(int percent)
{
    if (stream) {
        SDL_SetAudioStreamGain(stream.get(), percent / 100.0f);
    }
}

void GameAudio::play(SoundCue cue)
{
    const auto* path = cuePath(cue);
    if (!stream || !path || SDL_GetAudioStreamGain(stream.get()) == 0) {
        return;
    }
    SDL_AudioSpec source{};
    Uint8* samples = nullptr;
    Uint32 length = 0;
    if (!SDL_LoadWAV(path, &source, &samples, &length)) {
        return;
    }
    Uint8* converted = nullptr;
    int convertedLength = 0;
    if (length <= static_cast<Uint32>(std::numeric_limits<int>::max()) &&
        SDL_ConvertAudioSamples(&source, samples, static_cast<int>(length), &playback,
                                &converted, &convertedLength)) {
        SDL_ClearAudioStream(stream.get());
        SDL_PutAudioStreamData(stream.get(), converted, convertedLength);
        SDL_FlushAudioStream(stream.get());
    }
    SDL_free(converted);
    SDL_free(samples);
}
