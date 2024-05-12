#pragma once

olc::sound::WaveEngine *soundEngine = nullptr;

void setSoundEngine(olc::sound::WaveEngine *engine)
{
    soundEngine = engine;
}

class Sound
{
private:
    int channel;
    bool played = false;
    bool didStop = false;
    olc::sound::Wave wave;
    olc::sound::PlayingWave playingWave = {};

public:
    Sound(std::string path, int channel = 0)
    {
        this->channel = channel;
        wave = {path};
    }

    void SetPlayed(bool played)
    {
        this->played = played;
    }

    void Play(bool repeat = false, bool reset = false)
    {
        if (IsPlaying() && !reset)
        {
            return;
        }
        else if (IsPlaying() && reset)
        {
            Stop();
        }

        played = true;
        this->playingWave = soundEngine->PlayWaveform(&wave, repeat, 1.0f);
    }

    void Stop()
    {
        didStop = true;
    }

    bool IsPlaying()
    {
        if (didStop)
        {
            return false;
        }

        if (!played)
        {
            return false;
        }

        return !this->playingWave->bFinished;
    }
};