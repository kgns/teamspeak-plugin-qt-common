#include "volume/dsp_volume.h"

#include "volume/db.h"

#include <cmath>
#include <limits>

const float GAIN_FADE_RATE = (400.0f);	// Rate to fade at (dB per second)

void DspVolume::process(short *samples, int frame_count, int channels)
{
    set_gain_current(fade_step(frame_count));
    do_process(samples, frame_count * channels);
}

float DspVolume::fade_step(int sample_count)
{
    // compute manual gain
    auto current_gain = gain_current();
    const auto desired_gain = gain_desired();
    if (muted())
    {
        float fade_step = (GAIN_FADE_RATE / m_sample_rate) * sample_count;
        if (current_gain < VOLUME_MUTED - fade_step)
            current_gain += fade_step;
        else if (current_gain > VOLUME_MUTED + fade_step)
            current_gain -= fade_step;
        else
            current_gain = VOLUME_MUTED;
    }
    else if (current_gain != desired_gain)
    {
        float fade_step = (GAIN_FADE_RATE / m_sample_rate) * sample_count;
        if (current_gain < desired_gain - fade_step)
            current_gain += fade_step;
        else if (current_gain > desired_gain + fade_step)
            current_gain -= fade_step;
        else
            current_gain = desired_gain;
    }
    return current_gain;
}

//! Apply volume (no need to care for channels)
void DspVolume::do_process(short *samples, int sample_count)
{
    const auto mix_gain = static_cast<float>(db2lin_alt2(gain_current()));
    for (auto i = decltype(sample_count){0}; i < sample_count; ++i)
    {
        const auto temp = lround(samples[i] * mix_gain);
        if (temp < std::numeric_limits<int16_t>::min())
            samples[i] = std::numeric_limits<int16_t>::min();
        else if (temp > std::numeric_limits<int16_t>::max())
            samples[i] = std::numeric_limits<int16_t>::max();
        else
            samples[i] = static_cast<int16_t>(temp);
    }
}
