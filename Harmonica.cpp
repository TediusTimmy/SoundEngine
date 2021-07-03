/*
Copyright (c) 2021, Thomas DiModica
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
   This is my example program, but the default instrument is the Harmonica from
   the OLC Write Your Own Software Synthesizer series.
*/

#define OLC_PGE_APPLICATION
#include "include/olcPixelGameEngine.h"
#define OLC_PGEX_SOUND
#include "include/olcPGEX_Sound.h"

#define TD_SOUND_IMPLEMENTATION
#include "SoundEngine.h"

class OscillatorHolder
 {
public:
   double gain;
   TD_SOUND::Oscillator oscillator;
   double harmonic;

   OscillatorHolder(double gain, TD_SOUND::Oscillator oscillator, double harmonic) : gain(gain), oscillator(oscillator), harmonic(harmonic) { }
 };

class CompoundOscillator : public TD_SOUND::OscillatorImpl
 {
private:
   std::vector<OscillatorHolder> oscillators;

public:
   CompoundOscillator(const std::vector<OscillatorHolder>& oscillators) : oscillators(oscillators) { }

   double note(double frequency, double time) const override
    {
      double result = 0.0;
      for (auto& osc : oscillators)
       {
         result += osc.gain * osc.oscillator.note(osc.harmonic * frequency, time);
       }
      return result;
    }
 };

double JavidSine(double val)
 {
   double x = val / TD_SOUND::M_TWOPI;
   x = x - std::floor(x);
   return 20.875 * x * (x - 0.5) * (x - 1.0);
 }

class SquareWaveWithLowFrequencyOscillations : public TD_SOUND::OscillatorImpl
 {
private:
   double LFOLoudness;
   double LFORate;

public:
   SquareWaveWithLowFrequencyOscillations(double loudness, double rate) : LFOLoudness(loudness), LFORate(rate) { }

   double note(double frequency, double time) const override
    {
       // I'm fairly certain the second frequency in this equation is an error.
      return std::copysign(1.0, JavidSine(frequency * TD_SOUND::M_TWOPI * time + LFOLoudness * frequency * JavidSine(LFORate * TD_SOUND::M_TWOPI * time)));
    }
 };

class SawWaveWithLowFrequencyOscillations : public TD_SOUND::OscillatorImpl
 {
private:
   double LFOLoudness;
   double LFORate;

public:
   SawWaveWithLowFrequencyOscillations(double loudness, double rate) : LFOLoudness(loudness), LFORate(rate) { }

   double note(double frequency, double time) const override
    {
       // I'm fairly certain the second frequency in this equation is an error.
      double fundemental = frequency * TD_SOUND::M_TWOPI * time + LFOLoudness * frequency * JavidSine(LFORate * TD_SOUND::M_TWOPI * time);
      double sum = 0.0;
      for (int n = 1; n < 100; ++n)
       {
         sum += JavidSine(n * fundemental) / n;
       }
      return sum;
    }
 };

class ADSREnvelope : public TD_SOUND::EnvelopeImpl
 {
private:
   double attackPeak;
   double attackLength;
   double decayLength;
   double sustainLevel;
   double releaseLength;

public:
   ADSREnvelope() : attackPeak(1.0), attackLength(0.1), decayLength(0.1), sustainLevel(0.2), releaseLength(0.2) { }
   ADSREnvelope(double attackPeak, double attackLength, double decayLength, double sustainLevel, double releaseLength) :
      attackPeak(attackPeak), attackLength(attackLength), decayLength(decayLength), sustainLevel(sustainLevel), releaseLength(releaseLength) { }

   double loud(double time, double releaseTime) const override
    {
      double result = 0.0;
      if (-1.0 == releaseTime) //The note hasn't been released yet.
       {
         if (time < attackLength)
          {
            result = (time / attackLength) * attackPeak;
          }
         else if (time < (attackLength + decayLength))
          {
            result = attackPeak - ((time - attackLength) / decayLength) * (attackPeak - sustainLevel);
          }
         else
          {
            result = sustainLevel;
          }
       }
      else
       {
         if (releaseTime < attackLength)
          {
            result = (time / attackLength) * attackPeak;
          }
         else if (releaseTime < (attackLength + decayLength))
          {
            result = attackPeak - ((time - attackLength) / decayLength) * (attackPeak - sustainLevel);
          }
         else
          {
            result = sustainLevel;
          }
         result = result * ((releaseTime + releaseLength - time) / releaseLength);
       }
      return result;
    }

   double release() const override
    {
      return releaseLength;
    }
 };

std::map<char, TD_SOUND::Instrument> buildInstrument()
 {
   static TD_SOUND::Instrument harmonica (TD_SOUND::Oscillator(std::make_shared<CompoundOscillator>(CompoundOscillator(
    {
      OscillatorHolder(0.3 * 1.0, TD_SOUND::Oscillator(std::make_shared<SawWaveWithLowFrequencyOscillations>(0.001, 5.0)), 0.5),
      OscillatorHolder(0.3 * 1.0, TD_SOUND::Oscillator(std::make_shared<SquareWaveWithLowFrequencyOscillations>(0.001, 5.0)), 1.0),
      OscillatorHolder(0.3 * 0.5, TD_SOUND::Oscillator::makeSquareWaveOscillator(), 2.0),
      OscillatorHolder(0.3 * 0.05, TD_SOUND::Oscillator::makeNoiseOscillator(), 4.0)
    }))), TD_SOUND::Envelope(std::make_shared<ADSREnvelope>(1.0, 0.0, 1.0, 0.95, 0.1)));

   std::map<char, TD_SOUND::Instrument> result;
   result.insert(std::make_pair('\0', harmonica));
   return result;
 }

class SoundPlayer : public olc::PixelGameEngine
 {
private:
   static double samples[512];
   static int sample;
   std::vector<std::string> soundString;
   double globalTime;
   bool started;

   static double MyCustomSynthFunction(int nChannel, double fGlobalTime, double fTimeStep)
    {
      double result = TD_SOUND::Venue::sdGetSample(nChannel, fGlobalTime, fTimeStep);

      // Add sample to list of previous samples for visualization
      samples[sample] = result;
      sample = (sample + 1 ) & 511;

      return result;
    }

public:
   SoundPlayer(const std::vector<std::string>& soundString) : soundString(soundString), globalTime(0.0), started(false)
    {
      sAppName = "Sound Player";
    }

   bool OnUserCreate() override
    {
      olc::SOUND::InitialiseAudio(44100, 1, 8, 512);
      olc::SOUND::SetUserSynthFunction(MyCustomSynthFunction);
      TD_SOUND::Venue::getInstance().addMusicCallback(std::bind(&SoundPlayer::OnMusicEnded, this));
      return true;
    }

   void OnMusicEnded (void)
    {
      // This should only be called if this was previously parsed successfully.
      TD_SOUND::Venue::getInstance().queueMusic(soundString, buildInstrument());
    }

   bool OnUserUpdate(float fElapsedTime) override
    {
      Clear(olc::BLUE);
      for (int i = 511; i >= 0; i--)
       {
         double thisSample = samples[(sample + i) & 511];
         DrawLine(64 + i, 240, 64 + i, 240 - (int)(thisSample * 200.0), olc::RED);
         if (0U != soundString.size())
          {
            DrawString(20, 470, soundString[0].substr(0U, 64U));
          }
       }
      globalTime += fElapsedTime;
      if ((5.0 < globalTime) && (false == started))
       {
         started = true;
         try
          {
            TD_SOUND::Venue::getInstance().queueMusic(soundString, buildInstrument());
          }
         catch (const std::invalid_argument& e)
          {
            if (0U == soundString.size())
             {
               soundString.push_back("");
             }
            soundString[0] = std::string("Parse Failed: ") + e.what();
          }
       }
      return true;
    }

   bool OnUserDestroy()
    {
      olc::SOUND::DestroyAudio();
      return true;
    }
 };

int    SoundPlayer::sample = 0;
double SoundPlayer::samples[512];

int main (int /*argc*/, char ** /*argv*/)
 {
   std::vector<std::string> voices;
    {
      std::string toPlay;
      std::ifstream music ("Music.txt");
      std::getline(music, toPlay);
      while (true == music.good())
      {
         if ((false == toPlay.empty()) && ('/' != toPlay[0]))
         {
            voices.push_back(toPlay);
         }
         std::getline(music, toPlay);
      }
    }
   SoundPlayer demo (voices);
   if (demo.Construct(640, 480, 2, 2))
    {
      demo.Start();
    }
   return 0;
 }
