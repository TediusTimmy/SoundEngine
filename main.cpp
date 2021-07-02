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

#define OLC_PGE_APPLICATION
#include "include/olcPixelGameEngine.h"
#define OLC_PGEX_SOUND
#include "include/olcPGEX_Sound.h"

#define TD_SOUND_IMPLEMENTATION
#include "SoundEngine.h"

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
      return true;
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
            TD_SOUND::Venue::getInstance().queueMusic(soundString);
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
   TD_SOUND::Venue::getInstance().toggleLoop();
   SoundPlayer demo (voices);
   if (demo.Construct(640, 480, 2, 2))
    {
      demo.Start();
    }
   return 0;
 }
