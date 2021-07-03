// VERSION 2.1
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


AUTHOR'S NOTE:
Building this code into your program ought to cause the constant
TD_SOUND::legalRequirement to be included, verbatim, in the resulting binary.
It is the author's intent that this shall suffice to satisfy bullet point two.

This constant is the text:
"A portion of this binary is licensed so as to require this notice:\n\n"
followed by the entire license text.
*/

#ifndef TD_SOUND_ENGINE_H
#define TD_SOUND_ENGINE_H

#include <cmath>
#include <functional>
#include <limits>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

namespace TD_SOUND
 {

   const std::vector<double>& getStandardTwelveToneEqualNotes();
   const std::vector<std::string>& getNoteNames();

   /*
      To implement your own oscillator, you need to write a class that implements OscillatorImpl.
      The Oscillator class is non-polymorphic and uses the PIMPL idiom to give Oscillators value-like semantics.
      We can pass around Oscillators by value without worrying about object slicing.
      It does mean that your oscillator must be immutable: it cannot have internal state that is modified after construction.
      It can have internal state. The RectangularWaveOscillator stores a duty cycle. That state cannot be modified, though.

      Currently, the only required part of this interface is the call to note.
      It takes the frequency to play and the note time that we are playing at.
      Frequency is in Hertz, time is in seconds.
    */
   class OscillatorImpl
    {
   public:
      virtual double note(double frequency, double time) const = 0;
      virtual ~OscillatorImpl();
    };

   class Oscillator
    {
   private:
      std::shared_ptr<OscillatorImpl> oscillator;

   public:
      Oscillator(const std::shared_ptr<OscillatorImpl>& oscillator);

      double note(double frequency, double time) const;

      static Oscillator makeSineWaveOscillator();
      static Oscillator makeTriangularWaveOscillator();
      static Oscillator makeSquareWaveOscillator();
      static Oscillator makeSawWaveOscillator();
      static Oscillator makeNoiseOscillator();
      static Oscillator makeRectangularWaveOscillator(double dutyCycle);
    };

   class EnvelopeImpl
    {
   public:
      virtual double loud(double time, double releaseTime) const = 0;
      virtual double release() const = 0; // Return the release length.
      virtual ~EnvelopeImpl();
    };

   class Envelope
    {
   private:
      std::shared_ptr<EnvelopeImpl> envelope;

   public:
      Envelope(const std::shared_ptr<EnvelopeImpl>& envelope);

      double loud(double time, double releaseTime) const;
      double release() const;

      static Envelope makeDefaultAREnvelope();
    };

   class Instrument
    {
   private:
      Oscillator oscillator;
      Envelope envelope;

   public:
      Instrument(Oscillator oscillator, Envelope envelope);

      double note(double frequency, double time, double releaseTime) const;
      double release() const;

      static Instrument makeSineWaveInstrument();
      static Instrument makeTriangularWaveInstrument();
      static Instrument makeSquareWaveInstrument();
      static Instrument makeSawWaveInstrument();
      static Instrument makeNoiseInstrument();
      static Instrument makeRectangularWaveInstrument(double dutyCycle);
    };

   class Note
    {
   private:
      Instrument instrument;
      double frequency;
      double duration;
      double volume;

      double startTime;

   public:
      Note(Instrument instrument, double frequency, double startTime, double duration, double volume);

      bool before (double time) const;
      bool after (double time) const;
      double play (double time) const;
    };

   /*
      Voice assumes that calls to play() will be non-decreasing.
    */
   class Voice
    {
   private:
      std::vector<Note> notes;
      size_t index;
      std::list<Note*> activeNotes;

      double endPlay(double time);

   public:
      Voice();
      Voice(const std::vector<Note> notes);

      // Get the current sample value, between -1.0 and 1.0, for the given global time.
      // How voices play notes currently constrains making an ADSR envelope:
      //    the release of one note can't overlap with the attack of the next note.
      double play (double time);
      double playActive(double time) const;
      bool finished() const;
      void loop();
    };

   const std::map<char, Instrument>& getDefaultInstrument();
   Voice buildVoiceFromString(const std::string& input, const std::map<char, Instrument>& instruments = getDefaultInstrument(), const std::vector<double>& pitches = getStandardTwelveToneEqualNotes());

   class Maestro
    {
   private:
      std::vector<Voice> choir;

   public:
      Maestro() = default;
      Maestro(const std::vector<std::string>& music, const std::map<char, Instrument>& instruments = getDefaultInstrument());
      Maestro(const std::vector<Voice>& choir);

      double play(double time);
      bool finished() const;
      void loop();
    };

   class Venue
    {
   private:
      std::list<Maestro> program;
      volatile bool stopPlaying;
      volatile bool looping;
      double internalTime;
      std::function<void(void)> hollaback;

      Venue();

   public:
      static Venue& getInstance();
      void queueMusic(const std::vector<std::string>& music, const std::map<char, Instrument>& instruments = getDefaultInstrument());
      void queueMusic(const Maestro& song);
      void clearQueue();
      void toggleLoop();
      void addMusicCallback(std::function<void(void)> callOnMusicDone);

      double getSample(int unused, double globalTime, double timeDelta);
      static double sdGetSample(int unused, double globalTime, double timeDelta);
      static float sfGetSample(int unused, float globalTime, float timeDelta);
    };

   extern const char * const legalRequirement;

#ifdef TD_SOUND_IMPLEMENTATION

   const char * const legalRequirement =
"A portion of this binary is licensed so as to require this notice:\n\n"
"Copyright (c) 2021, Thomas DiModica\n"
"All rights reserved.\n\n"
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions are met:\n\n"
"* Redistributions of source code must retain the above copyright notice, this\n"
"  list of conditions and the following disclaimer.\n\n"
"* Redistributions in binary form must reproduce the above copyright notice,\n"
"  this list of conditions and the following disclaimer in the documentation\n"
"  and/or other materials provided with the distribution.\n\n"
"* Neither the name of the copyright holder nor the names of its\n"
"  contributors may be used to endorse or promote products derived from\n"
"  this software without specific prior written permission.\n\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
"DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE\n"
"FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
"DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR\n"
"SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER\n"
"CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,\n"
"OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";

   // This is stolen from GCCs C++20 implementation. I am targeting C++17, however.
   static const double M_PI    = 3.141592653589793238462643383279502884L;
   static const double M_PI_2  = M_PI * 0.5;
   static const double M_TWOPI = M_PI * 2.0;

   double sineWave (double frequency, double time)
    {
      return std::sin(frequency * M_TWOPI * time);
    }

   double triangularWave (double frequency, double time)
    {
      return std::asin(std::sin(frequency * M_TWOPI * time)) / M_PI_2;
    }

   double squareWave (double frequency, double time)
    {
      return std::copysign(1.0, std::sin(frequency * M_TWOPI * time));
    }

   double sawWave (double frequency, double time)
    {
      double Time = frequency * time;
      return 2.0 * (Time - std::floor(Time + .5));
    }

   double noise (double frequency, double time)
    {
      // The same note played at the same time should produce the same noise.
      return 1.0 - 2.0 * (std::hash<double>{}(frequency * M_TWOPI * time) / static_cast<double>((std::numeric_limits<size_t>::max)()));
    }

   double rectangularWave (double frequency, double time, double duty)
    {
      double Time = frequency * M_TWOPI * time;
      double loc = Time - std::floor(Time / M_TWOPI) * M_TWOPI;
      return (loc <= (duty * M_TWOPI)) ? 1.0 : -1.0;
    }

   // The A above middle C : ISO standard 16
   static const double A440 = 440.0;

   static const int notesPerOctave = 12; // Don't change these.
   static const int octavesImplemented = 9; // Don't even dare.
   static const int totalNotes = notesPerOctave * octavesImplemented;

   std::vector<double> generateTwelveToneEqual (double AAboveMiddleC)
    {
      std::vector<double> notes (totalNotes, 0.0);
      double A = AAboveMiddleC / 16.0;
      for (int O = 0; O < octavesImplemented; ++O)
       {
         notes[O * notesPerOctave +  0] = A / std::pow(2, 9.0 / 12.0);
         notes[O * notesPerOctave +  1] = A / std::pow(2, 8.0 / 12.0);
         notes[O * notesPerOctave +  2] = A / std::pow(2, 7.0 / 12.0);
         notes[O * notesPerOctave +  3] = A / std::pow(2, 6.0 / 12.0);
         notes[O * notesPerOctave +  4] = A / std::pow(2, 5.0 / 12.0);
         notes[O * notesPerOctave +  5] = A / std::pow(2, 4.0 / 12.0);
         notes[O * notesPerOctave +  6] = A / std::pow(2, 3.0 / 12.0);
         notes[O * notesPerOctave +  7] = A / std::pow(2, 2.0 / 12.0);
         notes[O * notesPerOctave +  8] = A / std::pow(2, 1.0 / 12.0);
         notes[O * notesPerOctave +  9] = A;
         notes[O * notesPerOctave + 10] = A * std::pow(2, 1.0 / 12.0);
         notes[O * notesPerOctave + 11] = A * std::pow(2, 2.0 / 12.0);
         A *= 2.0;
       }
      return notes;
    }

   const std::vector<double>& getStandardTwelveToneEqualNotes()
    {
      static std::vector<double> notes = generateTwelveToneEqual(A440);
      return notes;
    }

   std::vector<std::string> generateNoteNames()
    {
      std::vector<std::string> result;
      for (int i = 0; i < octavesImplemented; ++i)
       {
         result.emplace_back("C" + std::to_string(i));
         result.emplace_back("C#" + std::to_string(i));
         result.emplace_back("D" + std::to_string(i));
         result.emplace_back("D#" + std::to_string(i));
         result.emplace_back("E" + std::to_string(i));
         result.emplace_back("F" + std::to_string(i));
         result.emplace_back("F#" + std::to_string(i));
         result.emplace_back("G" + std::to_string(i));
         result.emplace_back("G#" + std::to_string(i));
         result.emplace_back("A" + std::to_string(i));
         result.emplace_back("A#" + std::to_string(i));
         result.emplace_back("B" + std::to_string(i));
       }
      return result;
    }

   const std::vector<std::string>& getNoteNames()
    {
      static std::vector<std::string> names = generateNoteNames();
      return names;
    }

   OscillatorImpl::~OscillatorImpl() { }

   class SineWaveOscillator : public OscillatorImpl
    {
   public:
      double note(double frequency, double time) const override
       {
         return sineWave(frequency, time);
       }
    };

   class TriangularWaveOscillator : public OscillatorImpl
    {
   public:
      double note(double frequency, double time) const override
       {
         return triangularWave(frequency, time);
       }
    };

   class SquareWaveOscillator : public OscillatorImpl
    {
   public:
      double note(double frequency, double time) const override
       {
         return squareWave(frequency, time);
       }
    };

   class SawWaveOscillator : public OscillatorImpl
    {
   public:
      double note(double frequency, double time) const override
       {
         return sawWave(frequency, time);
       }
    };

   class NoiseOscillator : public OscillatorImpl
    {
   public:
      double note(double frequency, double time) const override
       {
         return noise(frequency, time);
       }
    };

   class RectangularWaveOscillator : public OscillatorImpl
    {
   private:
      double duty;

   public:
      RectangularWaveOscillator(double dutyCycle) : duty(dutyCycle) { }

      double note(double frequency, double time) const override
       {
         return rectangularWave(frequency, time, duty);
       }
    };

   Oscillator::Oscillator(const std::shared_ptr<OscillatorImpl>& oscillator) : oscillator(oscillator) { }

   double Oscillator::note(double frequency, double time) const
    {
      return oscillator->note(frequency, time);
    }

   Oscillator Oscillator::makeSineWaveOscillator()
    {
      static std::shared_ptr<OscillatorImpl> theSineWaveGenerator = std::make_shared<SineWaveOscillator>();
      return Oscillator(theSineWaveGenerator);
    }

   Oscillator Oscillator::makeTriangularWaveOscillator()
    {
      static std::shared_ptr<OscillatorImpl> theTriangularWaveGenerator = std::make_shared<TriangularWaveOscillator>();
      return Oscillator(theTriangularWaveGenerator);
    }

   Oscillator Oscillator::makeSquareWaveOscillator()
    {
      static std::shared_ptr<OscillatorImpl> theSquareWaveGenerator = std::make_shared<SquareWaveOscillator>();
      return Oscillator(theSquareWaveGenerator);
    }

   Oscillator Oscillator::makeSawWaveOscillator()
    {
      static std::shared_ptr<OscillatorImpl> theSawWaveGenerator = std::make_shared<SawWaveOscillator>();
      return Oscillator(theSawWaveGenerator);
    }

   Oscillator Oscillator::makeNoiseOscillator()
    {
      static std::shared_ptr<OscillatorImpl> theNoiseGenerator = std::make_shared<NoiseOscillator>();
      return Oscillator(theNoiseGenerator);
    }

   Oscillator Oscillator::makeRectangularWaveOscillator(double dutyCycle)
    {
      return Oscillator(std::make_shared<RectangularWaveOscillator>(dutyCycle));
    }

   EnvelopeImpl::~EnvelopeImpl() { }

   class AREnvelope : public EnvelopeImpl
    {
   private:
      double attackPeak;
      double attackLength;
      double releaseLength;

   public:
      AREnvelope() : attackPeak(1.0), attackLength(240.0 / (64 * 256) * 0.1), releaseLength(240.0 / (64 * 256) * 0.1) { }

      double loud(double time, double releaseTime) const override
       {
         double result = 0.0;
         if (-1.0 == releaseTime) //The note hasn't been released yet.
          {
            if (time < attackLength)
             {
               result = (time / attackLength) * attackPeak;
             }
            else
             {
               result = attackPeak;
             }
          }
         else
          {
            if (releaseTime < attackLength)
             {
               result = (time / attackLength) * attackPeak;
             }
            else
             {
               result = attackPeak;
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

   Envelope::Envelope(const std::shared_ptr<EnvelopeImpl>& envelope) : envelope(envelope) { }

   double Envelope::loud(double time, double releaseTime) const
    {
      return envelope->loud(time, releaseTime);
    }

   double Envelope::release() const
    {
      return envelope->release();
    }

   Envelope Envelope::makeDefaultAREnvelope()
    {
      static std::shared_ptr<EnvelopeImpl> defaultAR = std::make_shared<AREnvelope>();
      return Envelope(defaultAR);
    }

   Instrument::Instrument(Oscillator oscillator, Envelope envelope) : oscillator(oscillator), envelope(envelope) { }

   double Instrument::note(double frequency, double time, double releaseTime) const
    {
      return envelope.loud(time, releaseTime) * oscillator.note(frequency, time);
    }

   double Instrument::release() const
    {
      return envelope.release();
    }

   Instrument Instrument::makeSineWaveInstrument()
    {
      return Instrument(Oscillator::makeSineWaveOscillator(), Envelope::makeDefaultAREnvelope());
    }

   Instrument Instrument::makeTriangularWaveInstrument()
    {
      return Instrument(Oscillator::makeTriangularWaveOscillator(), Envelope::makeDefaultAREnvelope());
    }

   Instrument Instrument::makeSquareWaveInstrument()
    {
      return Instrument(Oscillator::makeSquareWaveOscillator(), Envelope::makeDefaultAREnvelope());
    }

   Instrument Instrument::makeSawWaveInstrument()
    {
      return Instrument(Oscillator::makeSawWaveOscillator(), Envelope::makeDefaultAREnvelope());
    }

   Instrument Instrument::makeNoiseInstrument()
    {
      return Instrument(Oscillator::makeNoiseOscillator(), Envelope::makeDefaultAREnvelope());
    }

   Instrument Instrument::makeRectangularWaveInstrument(double dutyCycle)
    {
      return Instrument(Oscillator::makeRectangularWaveOscillator(dutyCycle), Envelope::makeDefaultAREnvelope());
    }

   Note::Note(Instrument instrument, double frequency, double startTime, double duration, double volume) :
      instrument(instrument), frequency(frequency), duration(duration), volume(volume), startTime(startTime) { }

   bool Note::before (double time) const
    {
      return time < startTime;
    }

   bool Note::after (double time) const
    {
      return time > (startTime + duration + instrument.release());
    }

   double Note::play (double time) const
    {
      double noteTime = time - startTime;
      return volume * instrument.note(frequency, noteTime, ((noteTime < duration) ? -1.0 : duration));
    }

   Voice::Voice() : notes(), index(0U), activeNotes() { }
   Voice::Voice(const std::vector<Note> notes) : notes(notes), index(0U), activeNotes() { }

   double Voice::endPlay(double time)
    {
      double result = playActive(time);
      activeNotes.remove_if([=](const Note* note) { return note->after(time); });
      return result;
    }

   double Voice::play (double time)
    {
      // Skip all passed notes.
      while ((index < notes.size()) && (true == notes[index].after(time)))
       {
         ++index;
       }
      // Exit if we are done.
      if (index == notes.size())
       {
         return endPlay(time);
       }
      // If this note hasn't started, we are resting.
      if ((index < notes.size()) && (true == notes[index].before(time)))
       {
         return endPlay(time);
       }
      // We must be playing this note right now.
      while ((index < notes.size()) && (false == notes[index].before(time)))
       {
         activeNotes.push_back(&(notes[index]));
         ++index;
       }
      return endPlay(time);
    }

   double Voice::playActive(double time) const
    {
      double sum = 0.0;
      for (const Note * note : activeNotes)
       {
         sum += note->play(time);
       }
      return sum;
    }

   bool Voice::finished() const
    {
      return (index == notes.size() && (0U == activeNotes.size()));
    }

   void Voice::loop()
    {
      index = 0U;
      activeNotes.clear();
    }

   std::map<char, Instrument> makeDefaultInstrument()
    {
      std::map<char, Instrument> result;
      result.insert(std::make_pair('\0', Instrument::makeSquareWaveInstrument()));
      return result;
    }

   const std::map<char, Instrument>& getDefaultInstrument()
    {
      static const std::map<char, Instrument> instruments = makeDefaultInstrument();
      return instruments;
    }

   class StringProcessor
    {
   public:
      const std::string& input;
      size_t location;
      char peeked;

      StringProcessor(const std::string& input) : input(input), location(0U), peeked('\0')
       {
         if (0U != input.length())
          {
            peeked = std::toupper(static_cast<unsigned char>(input[0]));
            // std::isspace doesn't return true, it returns not false
            if (false != std::isspace(static_cast<unsigned char>(peeked)))
             {
               peeked = getNextChar();
             }
          }
       }

      bool done () const
       {
         return location == input.length();
       }

      char getNextChar()
       {
         bool gotNext = false;
         char result = '\0';
         while (false == gotNext)
          {
            ++location;
            if (location == input.length())
             {
               gotNext = true;
             }
            else if (false == std::isspace(static_cast<unsigned char>(input[location])))
             {
               gotNext = true;
               result = std::toupper(static_cast<unsigned char>(input[location]));
             }
          }
         return result;
       }

      char peek () const
       {
         return peeked;
       }

      char consume ()
       {
         char result = peeked;
         peeked = getNextChar();
         return result;
       }

      int getNumber ()
       {
         int number = 0;
         bool firstChar = true;
         bool keepLooking = true;
         while (true == keepLooking)
          {
            switch (peek())
             {
               case '0':
               case '1':
               case '2':
               case '3':
               case '4':
               case '5':
               case '6':
               case '7':
               case '8':
               case '9':
                  number = number * 10 + (consume() - '0');
                  break;
               default:
                  if (true == firstChar)
                   {
                     throw std::invalid_argument("Command requires value, none given.");
                   }
                  keepLooking = false;
                  break;
             }
            firstChar = false;
          }
         return number;
       }
    };

   Voice buildVoiceFromString(const std::string& input, const std::map<char, Instrument>& instruments, const std::vector<double>& pitches)
    {
      static const int map [] = { 9, 11, 0, 2, 4, 5, 7 };

      int currentOctave = 4;
      int currentBeatNote = 4;
      int currentTempo = 120;
      double articulation = 7.0 / 8.0;
      // Length in seconds of each note. There are 60 seconds in a minute, but tempo is in quarter notes per minute.
      //          sec / 4minute      beats / note      note / 4minute
      double noteLength = 240.0 / (currentBeatNote * currentTempo);
      double volume = 0.5;
      double time = 0.0;

      std::vector<Note> notes;

      if (totalNotes != static_cast<int>(pitches.size()))
       {
         throw std::invalid_argument("Note array of invalid size.");
       }

      if (instruments.find('\0') == instruments.end())
       {
         throw std::invalid_argument("No default instrument in instrument list.");
       }
      Instrument instrument = instruments.find('\0')->second;

      StringProcessor command (input);

      while (false == command.done())
       {
         switch (command.peek())
          {
         case 'A':
         case 'B':
         case 'C':
         case 'D':
         case 'E':
         case 'F':
         case 'G':
          {
            int in = map[command.consume() - 'A'];
            int note = currentOctave * notesPerOctave + in;

            double tempDuration = articulation;
            double tempLength = noteLength;
            double tempVolume = volume;

            bool modifiers = true;
            bool advance = true;
            double nextDot = tempLength * 0.5;
            while (true == modifiers)
             {
               switch (command.peek())
                {
               case '+':
               case '#':
                  command.consume();
                  ++note;
                  if (totalNotes == note)
                   {
                     throw std::invalid_argument("Tried to sharp the highest note.");
                   }
                  break;
               case '-':
                  command.consume();
                  --note;
                  if (-1 == note)
                   {
                     throw std::invalid_argument("Tried to flat the lowest note.");
                   }
                  break;
               case '.':
                  command.consume();
                  tempLength += nextDot;
                  nextDot *= 0.5;
                  break;
               case '1': // This modifier/suffix overrides the length of the note.
               case '2': // It MUST occur before a '.'.
               case '3':
               case '4':
               case '5':
               case '6':
               case '7':
               case '8':
               case '9':
                {
                  int length = command.getNumber();
                  if ((length < 1) || (length > 64))
                   {
                     throw std::invalid_argument("Invalid note length.");
                   }
                  tempLength = 240.0 / (length * currentTempo);
                  nextDot = tempLength * 0.5;
                }
                  break;
               case '_':
                  command.consume();
                  tempDuration = 1.0;
                  break;
               case '\'':
                  command.consume();
                  tempDuration = 3.0 / 4.0;
                  break;
               case '^':
                  command.consume();
                  tempVolume = std::min(tempVolume + 0.125, 1.0);
                  break;
               case ',':
                  command.consume();
                  modifiers = false;
                  advance = false;
                  break;
               default:
                  modifiers = false;
                  break;
                }
             }

            notes.emplace_back(instrument, pitches[note], time, tempLength * tempDuration, tempVolume);
            if (true == advance)
             {
               time += tempLength;
             }
          }
            break;

         case '>':
            command.consume();
            ++currentOctave;
            if (octavesImplemented == currentOctave)
             {
               throw std::invalid_argument("Operation '>' exceeded octave range.");
             }
            break;

         case '<':
            command.consume();
            --currentOctave;
            if (-1 == currentOctave)
             {
               throw std::invalid_argument("Operation '<' exceeded octave range.");
             }
            break;

         case 'T':
            command.consume();
            currentTempo = command.getNumber();
            if ((currentTempo < 16) || (currentTempo > 256))
             {
               throw std::invalid_argument("Asked to play music either too slow or too fast.");
             }
            noteLength = 240.0 / (currentBeatNote * currentTempo);
            break;

         case 'L':
            command.consume();
            currentBeatNote = command.getNumber();
            if ((currentBeatNote < 1) || (currentBeatNote > 64))
             {
               throw std::invalid_argument("Invalid note length.");
             }
            noteLength = 240.0 / (currentBeatNote * currentTempo);
            break;

         case 'O':
            command.consume();
            currentOctave = command.getNumber();
            if (octavesImplemented <= currentOctave)
             {
               throw std::invalid_argument("Set current octave too high.");
             }
            break;

         case 'N':
          {
            command.consume();
            int note = command.getNumber();
            if (note > totalNotes) // We will subtract one from note.
             {
               throw std::invalid_argument("Invalid note number.");
             }
            if (0 != note)
             {
               notes.emplace_back(instrument, pitches[note - 1], time, noteLength * articulation, volume);
             }
            time += noteLength;
          }
            break;

         case 'P':
         case 'R': // Because "pauses" are RESTS
          {
            command.consume();
            double tempLength = noteLength;
            // Allow no length to be specified, to indicate using the current note length, just like for notes.
            if ((command.peek() >= '0') && (command.peek() <= '9'))
            {
               int length = command.getNumber();
               // I have a specification that says that a zero length here is a no-op. I don't like that.
               if ((length < 1) || (length > 64))
                {
                  throw std::invalid_argument("Invalid note length.");
                }
               tempLength = 240.0 / (length * currentTempo);
            }
            double nextDot = tempLength * 0.5;
            while ('.' == command.peek())
             {
               command.consume();
               tempLength += nextDot;
               nextDot *= 0.5;
             }
            time += tempLength;
          }
            break;

         case 'M':
            command.consume();
            switch (command.peek())
             {
            case 'F':
            case 'B':
               // No support for foreground or background music.
               command.consume();
               break;
            case 'L':
               command.consume();
               articulation = 1.0;
               break;
            case 'N':
               command.consume();
               articulation = 7.0 / 8.0;
               break;
            case 'S':
               command.consume();
               articulation = 3.0 / 4.0;
               break;
            default:
               throw std::invalid_argument(std::string("Did not understand music ('M') command component \'") + command.peek() + "\'.");
             }
            break;

         case 'I':
            command.consume();
            switch(command.peek())
             {
            case 'Q':
               command.consume();
               instrument = Instrument::makeSquareWaveInstrument();
               break;
            case 'T':
               command.consume();
               instrument = Instrument::makeTriangularWaveInstrument();
               break;
            case 'S':
               command.consume();
               instrument = Instrument::makeSineWaveInstrument();
               break;
            case 'W':
               command.consume();
               instrument = Instrument::makeSawWaveInstrument();
               break;
            case 'N':
               command.consume();
               instrument = Instrument::makeNoiseInstrument();
               break;
            case 'X':
               command.consume();
               if (instruments.end() == instruments.find(command.peek()))
                {
                  throw std::invalid_argument("Invalid instrument.");
                }
               instrument = instruments.find(command.consume())->second;
               break;
            case 'P':
             {
               command.consume();
               int dutyCycle = command.getNumber();
               if ((dutyCycle < 1) || (dutyCycle > 99))
                {
                  throw std::invalid_argument("Invalid duty cycle for a rectangular wave.");
                }
               instrument = Instrument::makeRectangularWaveInstrument(dutyCycle / 100.0);
             }
               break;
            default:
               throw std::invalid_argument("Invalid instrument.");
             }
            break;

         case 'V':
            command.consume();
            switch(command.peek())
             {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
             {
               int newVolume = command.getNumber();
               if (100 < newVolume)
                {
                  throw std::invalid_argument("Invalid volume.");
                }
               volume = newVolume / 100.0;
             }
               break;
            case 'P': // paino
               command.consume();
               if ('P' == command.peek()) // pianissimo
                {
                  command.consume();
                  if ('P' == command.peek()) // pianississimo
                   {
                     command.consume();
                     volume = .125;
                   }
                  else
                   {
                     volume = .25;
                   }
                }
               else
                {
                  volume = .375;
                }
               if (';' == command.peek()) // Allow this to be followed by a ';' to allow playing a rest after changing the volume.
                {
                  command.consume();
                }
               break;
            case 'M':
               command.consume();
               if ('P' == command.peek()) // mezzo-piano
                {
                  command.consume();
                  volume = 0.5;
                }
               else if ('F' == command.peek()) // mezzo-forte
                {
                  command.consume();
                  volume = .625;
                }
               else
                {
                  throw std::invalid_argument("Invalid volume specification: mezzo-I-don't-know.");
                }
               if (';' == command.peek()) // For consistency
                {
                  command.consume();
                }
               break;
            case 'F':
               command.consume();
               if ('F' != command.peek()) // forte
                {
                  volume = .75;
                }
               else
                {
                  command.consume();
                  if ('F' != command.peek()) // fortissimo
                   {
                     volume = .875;
                   }
                  else // fortississimo
                   {
                     command.consume();
                     volume = 1.0;
                   }
                }
               if (';' == command.peek()) // Allow this to be followed by a ';' to allow playing an F after changing the volume.
                {
                  command.consume();
                }
               break;
            default:
               throw std::invalid_argument("Invalid volume specification.");
             }
            break;

         default:
            throw std::invalid_argument(std::string("Did not understand command component \'") + command.peek() + "\'.");
          }
       }

      return Voice(notes);
    }

   Maestro::Maestro(const std::vector<std::string>& music, const std::map<char, Instrument>& instruments) : choir()
    {
      for (auto& voice : music)
       {
         choir.emplace_back(buildVoiceFromString(voice, instruments));
         if (true == choir.back().finished()) // Throw out empty voices.
          {
            choir.pop_back();
          }
       }
    }

   Maestro::Maestro(const std::vector<Voice>& choir) : choir(choir) { }

   double Maestro::play(double time)
    {
      double sample = 0.0;
      if (0U != choir.size())
       {
         for (auto& voice : choir)
         {
            sample += voice.play(time);
         }
         sample /= choir.size();
       }
      return sample;
    }

   bool Maestro::finished() const
    {
      bool result = true;
      for (auto& voice : choir)
       {
         result &= voice.finished();
       }
      return result;
    }

   void Maestro::loop()
    {
      for (auto& voice : choir)
       {
         voice.loop();
       }
    }

   Venue::Venue() : program(), stopPlaying(false), looping(false), internalTime(-1.0), hollaback(nullptr) { }

   Venue& Venue::getInstance()
    {
      static Venue instance;
      return instance;
    }

   void Venue::queueMusic(const std::vector<std::string>& music, const std::map<char, Instrument>& instruments)
    {
      program.emplace_back(music, instruments);
    }

   void Venue::queueMusic(const Maestro& song)
    {
      program.push_back(song);
    }

   void Venue::clearQueue()
    {
      stopPlaying = true;
    }

   void Venue::toggleLoop()
    {
      looping = !looping;
    }

   void Venue::addMusicCallback(std::function<void(void)> callOnMusicDone)
    {
      hollaback = callOnMusicDone;
    }

   double Venue::getSample(int unused, double /*globalTime*/, double timeDelta)
    {
      if (0 != unused) // Is this the wrong channel?
       {
         return 0.0;
       }
      if (true == stopPlaying) // Have we been told to stop?
       {
         program.clear();
         stopPlaying = false;
         internalTime = -1.0;
         if (nullptr != hollaback) // Should I tell someone about this?
          {
            hollaback();
          }
       }
      if (0U == program.size()) // Is there nothing to play?
       {
         return 0.0;
       }
      if (true == program.front().finished()) // Has the most recent song ended?
       {
         if (true == looping) // But, is it looping?
          {
            program.front().loop();
          }
         else
          {
            program.pop_front();
          }
         internalTime = -1.0;
       }
      if ((0U == program.size()) && (nullptr != hollaback)) // Should I tell someone to fill the queue?
       {
         hollaback();
       }
      if (0U == program.size()) // Is there NOW nothing to play?
       {
         return 0.0;
       }
      if (-1.0 == internalTime) // Have we just started playing this song?
       {
         internalTime = 0.0;
       }
      else
       {
         internalTime += timeDelta;
       }
      return program.front().play(internalTime);
    }

   double Venue::sdGetSample(int unused, double globalTime, double timeDelta)
    {
      return getInstance().getSample(unused, globalTime, timeDelta);
    }

   float Venue::sfGetSample(int unused, float globalTime, float timeDelta)
    {
      return getInstance().getSample(unused, globalTime, timeDelta);
    }

#endif /* TD_SOUND_IMPLEMENTATION */

 } // namespace TD_SOUND

#endif /* TD_SOUND_ENGINE_H */
