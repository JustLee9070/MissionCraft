
#include "GameAudio.h"

#include <algorithm>
#include <limits>

namespace {
constexpr SDL_AudioSpec playback{SDL_AUDIO_S16, 2, 44100};

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
        SDL_Log("Audio initialization failed: %s", SDL_GetError());
        return false;
    }

    stream.reset(SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &playback, nullptr, nullptr));

    if (!stream) {
        SDL_Log("SFX audio stream creation failed: %s", SDL_GetError());
        return false;
    }

    if (!SDL_ResumeAudioStreamDevice(stream.get())) {
        SDL_Log("SFX audio stream resume failed: %s", SDL_GetError());
        stream.reset();
        return false;
    }

    musicStream.reset(SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &playback, nullptr, nullptr));

    if (!musicStream) {
        SDL_Log("Music audio stream creation failed: %s", SDL_GetError());
    } else {
        SDL_SetAudioStreamGain(musicStream.get(), musicVolume / 100.0f);

        if (!SDL_ResumeAudioStreamDevice(musicStream.get())) {
            SDL_Log("Music audio stream resume failed: %s", SDL_GetError());
            musicStream.reset();
        }
    }

    setVolume(sfxVolume);
    return true;
}

void GameAudio::setVolume(int percent)
{
    sfxVolume = std::clamp(percent, 0, 100);

    if (stream) {
        SDL_SetAudioStreamGain(stream.get(), sfxVolume / 100.0f);
    }
}

void GameAudio::setMusicVolume(int percent)
{
    musicVolume = std::clamp(percent, 0, 100);

    if (musicStream) {
        SDL_SetAudioStreamGain(musicStream.get(), musicVolume / 100.0f);
    }
}

void GameAudio::play(SoundCue cue)
{
    const auto* path = cuePath(cue);

    if (!stream || !path || sfxVolume == 0) {
        return;
    }

    SDL_AudioSpec source{};
    Uint8* samples = nullptr;
    Uint32 length = 0;

    if (!SDL_LoadWAV(path, &source, &samples, &length)) {
        SDL_Log("Failed to load sound effect %s: %s", path, SDL_GetError());
        return;
    }

    Uint8* converted = nullptr;
    int convertedLength = 0;

    if (length <= static_cast<Uint32>(std::numeric_limits<int>::max())) {
        if (SDL_ConvertAudioSamples(&source, samples, static_cast<int>(length),
                                    &playback, &converted, &convertedLength)) {
            SDL_ClearAudioStream(stream.get());
            SDL_PutAudioStreamData(stream.get(), converted, convertedLength);
            SDL_FlushAudioStream(stream.get());
        } else {
            SDL_Log("Sound effect conversion failed: %s", SDL_GetError());
        }
    }

    SDL_free(converted);
    SDL_free(samples);
}

bool GameAudio::playMusic(const char* path)
{
    if (!musicStream || !path) {
        SDL_Log("Cannot play music: audio stream or path is invalid.");
        return false;
    }

    stopMusic();

    SDL_AudioSpec source{};
    Uint8* samples = nullptr;
    Uint32 length = 0;

    if (!SDL_LoadWAV(path, &source, &samples, &length)) {
        SDL_Log("Failed to load music: %s", SDL_GetError());
        SDL_Log("Attempted path: %s", path);
        return false;
    }

    Uint8* converted = nullptr;
    int convertedLength = 0;
    bool success = false;

    if (length > static_cast<Uint32>(std::numeric_limits<int>::max())) {
        SDL_Log("Music file is too large to convert.");
    } else if (!SDL_ConvertAudioSamples(&source, samples, static_cast<int>(length),
                                        &playback, &converted, &convertedLength)) {
        SDL_Log("Music conversion failed: %s", SDL_GetError());
    } else if (convertedLength > 0) {
        musicSamples.assign(converted, converted + convertedLength);
        musicPosition = 0;
        success = true;
    }

    SDL_free(converted);
    SDL_free(samples);

    if (!success) {
        musicSamples.clear();
        return false;
    }

    SDL_ClearAudioStream(musicStream.get());

    if (!SDL_SetAudioStreamGetCallback(musicStream.get(), feedMusic, this)) {
        SDL_Log("Failed to set music callback: %s", SDL_GetError());
        musicSamples.clear();
        return false;
    }

    SDL_SetAudioStreamGain(musicStream.get(), musicVolume / 100.0f);

    if (!SDL_ResumeAudioStreamDevice(musicStream.get())) {
        SDL_Log("Failed to resume music playback: %s", SDL_GetError());
        stopMusic();
        return false;
    }

    SDL_Log("Background music started: %s", path);
    return true;
}

void SDLCALL GameAudio::feedMusic(void* userdata, SDL_AudioStream* audioStream,
                                  int additionalAmount, int)
{
    auto* audio = static_cast<GameAudio*>(userdata);

    if (!audio || audio->musicSamples.empty() || additionalAmount <= 0) {
        return;
    }

    size_t remaining = static_cast<size_t>(additionalAmount);

    while (remaining > 0) {
        size_t available = audio->musicSamples.size() - audio->musicPosition;
        size_t amount = std::min(remaining, available);

        if (!SDL_PutAudioStreamData(
                audioStream,
                audio->musicSamples.data() + audio->musicPosition,
                static_cast<int>(amount))) {
            SDL_Log("Failed to feed music stream: %s", SDL_GetError());
            break;
        }

        audio->musicPosition += amount;
        remaining -= amount;

        if (audio->musicPosition >= audio->musicSamples.size()) {
            audio->musicPosition = 0;
        }
    }
}

void GameAudio::stopMusic()
{
    if (!musicStream) {
        musicSamples.clear();
        musicPosition = 0;
        return;
    }

    SDL_SetAudioStreamGetCallback(musicStream.get(), nullptr, nullptr);
    SDL_ClearAudioStream(musicStream.get());

    musicSamples.clear();
    musicPosition = 0;
}
