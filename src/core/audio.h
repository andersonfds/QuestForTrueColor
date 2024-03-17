#pragma once

void LoadMusic(std::string path, float volume = 0.5f)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    std::cout << "SDL initialized" << std::endl;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return;
    }

    Mix_Music *music = Mix_LoadMUS(path.c_str());
    if (music == NULL)
    {
        std::cout << "Failed to load beat music! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return;
    }

    Mix_PlayMusic(music, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME * volume);
}

void StopMusic()
{
    Mix_HaltMusic();
}

void ClearMusic()
{
    Mix_HaltMusic();
    Mix_CloseAudio();
}

Mix_Chunk *LoadSfx(std::string path)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return NULL;
    }

    std::cout << "SDL initialized" << std::endl;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return NULL;
    }

    return Mix_LoadWAV(path.c_str());
}

void StopSfx(int channel)
{
    Mix_HaltChannel(channel);
}

class Sound
{
private:
    Mix_Chunk *sfx;
    int channel;
    bool played = false;

public:
    Sound(std::string path, int channel = 0)
    {
        this->channel = channel;
        sfx = LoadSfx(path);
        if (sfx == NULL)
        {
            std::cout << "Failed to load beat music! SDL_mixer Error: " << Mix_GetError() << std::endl;
            return;
        }
    }

    void SetPlayed(bool played)
    {
        this->played = played;
    }

    void Play(bool repeat = false, bool reset = false)
    {
        if (played && !reset)
        {
            return;
        }

        played = true;

        if (sfx == NULL)
        {
            std::cout << "Failed to load beat music! SDL_mixer Error: " << Mix_GetError() << std::endl;
            return;
        }

        if (IsPlaying() && !reset)
        {
            return;
        }
        else if (IsPlaying() && reset)
        {
            Stop();
        }

        int repeatValue = repeat ? -1 : 0;
        Mix_PlayChannel(channel, sfx, repeatValue);
        Mix_Volume(channel, MIX_MAX_VOLUME);
    }

    void Stop()
    {
        // Mix_HaltChannel(channel);
    }

    ~Sound()
    {
        Mix_FreeChunk(sfx);
    }

    void SetVolume(float volume)
    {
        int playVolume = MIX_MAX_VOLUME * volume;
        Mix_Volume(channel, playVolume);
    }

    bool IsPlaying()
    {
        return Mix_Playing(channel);
    }
};