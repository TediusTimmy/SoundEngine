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

#define TD_SOUND_IMPLEMENTATION
#include "SoundEngine.h"

#include <fstream>
#include <string>
#include <iostream>

int main (int argc, char ** argv)
 {
   if (argc != 3)
    {
      std::cout << "MakeWave version 1.0 : Copyright 2021 Thomas DiModica" << std::endl <<
         "usage: MakeWave <input file> <output file>" << std::endl <<
         "MakeWave converts text music in Music Markup Language to WAV files." << std::endl << std::endl;
      return 1;
    }

   std::vector<std::string> voices;
    {
      std::string toPlay;
      std::ifstream music (argv[1]);
      if (false == music.good())
       {
         std::cerr << "Error opening file: " << argv[1] << std::endl;
         return 2;
       }
      toPlay = "";
      std::getline(music, toPlay);
      while (true == music.good())
       {
         if ((false == toPlay.empty()) && ('/' != toPlay[0]))
          {
            voices.push_back(toPlay);
          }
         toPlay = "";
         std::getline(music, toPlay);
       }
      if (false == toPlay.empty())
       {
         voices.push_back(toPlay);
       }
    }
   if (true == voices.empty())
    {
      std::cerr << "Error reading file, file contained no text: " << argv[1] << std::endl;
      return 2;
    }

   try
    {
      TD_SOUND::Venue::getInstance().queueMusic(voices);
    }
   catch (const std::invalid_argument& e)
    {
      std::cerr << "Error parsing music file: " << e.what() << std::endl;
      return 3;
    }

   bool done = false;
   TD_SOUND::Venue::getInstance().addMusicCallback([&](){ done = true; });

   const int samplerate = 44100;
   const double step = 1.0 / samplerate;
   std::vector<short> music;
    {
      size_t sample = 0U;

      while (false == done)
       {
         double curTime = static_cast<double>(sample) / samplerate;
         double thisSample = std::clamp(TD_SOUND::Venue::sdGetSample(0, curTime, step), -1.0, 1.0);
         short curSample = static_cast<short>(thisSample * (std::numeric_limits<short>::max)());
         music.push_back(curSample);
       }
    }

   std::cout << "Voices found (empty voices are counted here, but may have been removed): " << voices.size() << std::endl <<
      "Samples generated: " << music.size() << std::endl <<
      "Length: " << (static_cast<double>(music.size()) / samplerate) << std::endl;

    {
      std::ofstream fileout (argv[2], std::ios::out | std::ios::binary);
      if (false == fileout.good())
       {
         std::cerr << "Error opening file: " << argv[1] << std::endl;
         return 4;
       }
      fileout.write("RIFF", 4);
      unsigned int samplesSize = 2U * static_cast<unsigned int>(music.size());
      unsigned int dataLength = 36U + samplesSize;
      fileout.write(static_cast<char*>(static_cast<void*>(&dataLength)), 4);
      fileout.write("WAVE", 4);
      fileout.write("fmt ", 4);
      fileout.write("\x10\0\0\0", 4); // Size of header : 16
      fileout.write("\1\0\1\0", 4); // Format : 1 = PCM, Channels : 1
      fileout.write(static_cast<const char*>(static_cast<const void*>(&samplerate)), 4);
      unsigned int byterate = 1U /*num channels*/ * static_cast<unsigned int>(samplerate) * 2U /* bytes per sample */;
      fileout.write(static_cast<char*>(static_cast<void*>(&byterate)), 4);
      fileout.write("\2\0\x10\0", 4); // Block align : 1 channel * 2 bytes per sample, Bits per sample : 16
      fileout.write("data", 4);
      fileout.write(static_cast<char*>(static_cast<void*>(&samplesSize)), 4);
      fileout.write(static_cast<char*>(static_cast<void*>(&(music[0]))), samplesSize);
    }

   return 0;
 }
