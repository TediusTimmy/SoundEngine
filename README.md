TD Sound Engine
===============

An implementation of Music Macro Language (from QBasic and other BASICs) for Pixel Game Engine.

VERSION 2

Usage
-----

I've tried to follow David's lead in making this easy to use: there is just the one header file, and one needs to define `TD_SOUND_IMPLEMENTATION` before including `SoundEngine.h` in at least one compiled file. As a result of trying to make this easy to integrate, I've done something that I'm not particularly happy with. Including this project will cause the following string to be compiled into your program: `"A portion of this binary is licensed so as to require this notice:\n\n"` This is followed by the Three-Clause BSD license that the code is licensed under. The purpose of this is to make it easy: including the header also performs all of the legal requirements of using the code that the license requires. The copyright statement and license are automagically included in your program, so you, the library user, don't have to do anything. It just feels a little unsavory to me.

Anyway, using it should be almost exactly like `olcPixelGameEngine.h`. I must also note that it is written for `olcPGEX_Sound.h`, however, how time is handled works around the bug in how `olc::SOUND` handles time.

If you just want it to handle background music, you can have a line like this somewhere: `olc::SOUND::SetUserSynthFunction(TD_SOUND::Venue::sfGetSample);` The whole music system is written in terms of the custom synthesizer function of `olc::SOUND`. If that collides with your own synthesizer, I'm sorry. You are smart enough to use it anyway.

To actually play music: call `TD_SOUND::Venue::getInstance().queueMusic()` passing in a `std::vector` of `std::string`. Each string is expected to be a complete song for one voice. All voices will be played simultaneously (and scaled by the number of voices to attempt to level the volume). Polyphony is achieved through multiple voices. The queueMusic function will throw a `std::invalid_argument` exception if the string cannot be parsed, and will remove any voice that doesn't make sound.

The song at the front of the queue can be looped using `TD_SOUND::Venue::getInstance().toggleLoop()`.
Also, you can install a callback for when the last song in the queue ends: `TD_SOUND::Venue::getInstance().addMusicCallback()`. The called function takes no arguments and will not return anything.

### Example Program

To get the example program to work, copy any of the examples in the SampleMusic directory and put it in the same directory as the compiled program. Then, rename it "Music.txt".  
None of that music is mine. I didn't create any of it. I'm just using it as an example of what the engine is capable of.

Music Macro Language
--------------------

The Music Macro Language is a string representation of music. You can find many references on the Internet as to the definition, but here it is again, noting all of the implementation-specific details.

| Character | Description |
|-----------|-------------|
| `O`n       | Set the current octave to 'n', where n is 0 - 8. In this implementation, O4 is QBasic's O2. I added two octaves of bass in order to make the octave range cover that of a standard 88-key piano. |
| `<`        | Move down an octave. It is an error to try to move down when the current octave is 0. |
| `>`        | Move up an octave. It is an error to try to move up when the current octave is 8. |
| `A` - `G`  | Play the specified note in the current octave. Notes played this way can have modifiers, explained later, applied to them. |
| `N`nnn     | Play note nnn, where n is 0 - 108. Note 0 is a rest. This form cannot have modifiers, and is only implemented in case someone tries to use it. |
| `P` or `R` | Rest (Pause). Rests can have length-changing modifiers. |
| `MF`       | This does nothing. It is retained so that it does not have to be removed for old stolen music. |
| `MB`       | This does nothing. It is retained so that it does not have to be removed for old stolen music. |
| `ML`       | Music Legato. Each note will be played for the entire duration of the current beat. |
| `MN`       | Music Normal. Each note will be played for 7/8 of the current beat. |
| `MS`       | Music Staccato. Each note will be played for 3/4 of the current beat. |
| `L`nn      | Set the default length of each note. As in: 1 will be whole notes, 2 half notes, etc. This does allow irregular timings like triplets. This is the note that gets the beat, and can range from 1 to 64. |
| `T`nnn     | Set the tempo of the music, in quarter note beats per minute. This can be as slow as 16, and as fast as 256. |
| `V`nnn     | Set the volume between 0 and 100%. |
| `V`x`;`    | Set the volume to a preset. The semicolon is optional, and allows, for instance, having a rest after setting the volume to piano, or playing an F after setting the volume to forte. |
| `I`x       | Set the current instrument. See table below. |

| Modifier | Description |
|----------|-------------|
| `#` or `+` | Make the preceding note sharp. Each sharp will move a semitone up. It is an error to sharp O8 B. |
| `-`        | Make the preceding note flat. Each flat will move a semitone down. It is an error to flat O0 C. |
| nn         | Play the preceding note as an nn note, where n is from 1 to 64. This overrides the default length. 1 is a whole note, 4 a quarter note, etc. This implementation does not accept nn as 0. This modifier can be added to rests. As an implementation detail, it should occur before any dots. |
| `.`        | Play the preceding note 3/2 as long. When these stack, they add half the length that was previously added. So, `..` makes the note 7/4 as long. The series converges on twice as long, rather than diverges. This modifier can be added to rests. |
| `_`        | Play the preceding note tenuto, or for the entire duration of the current beat. |
| `'`        | Play the preceding note staccato, or for 3/4 of the current beat. |
| `^`        | Play the preceding note marcato. For this implementation, that means play it at the next higher volume level, or 12.5% louder. |
| `,`        | This modifier stops the processing of modifiers. It plays the following note at the same time as the current note. No sound leveling is applied to polyphony here. The last note determines when the next note will play. As in `L4O3C1,>C1,>CEGE` will play in the duration of one whole note. |

While pianississimo and fotrississimo are not, I feel, common, adding them gives me eight levels to work with.
| Sound Level | Description | Volume |
|-------------|-------------|--------|
| `PPP` | pianississimo | 12.5% |
| `PP`  | pianissimo    | 25%   |
| `P`   | piano         | 37.5% |
| `MP`  | mezzo-piano   | 50%   |
| `MF`  | mezzo-forte   | 62.5% |
| `F`   | forte         | 75%   |
| `FF`  | fortissimo    | 87.5% |
| `FFF` | fortississimo | 100%  |

| Instrument | Description |
|------------|-------------|
| `S`   | Sine Wave |
| `Q`   | Square Wave |
| `T`   | Triangular Wave |
| `W`   | Saw Wave (harsh synthetic version) |
| `N`   | Noise |
| `P`nn | Rectangular Wave with an nn% duty cycle, between 1 and 99. |


Custom Instruments
------------------

This final bit will be about custom instruments. After I actually watched the videos on writing a software synthesizer, I changed this architecture up a bit.

Now, I am following the advice that an instrument is an oscillator and an envelope. So, one needs to implement `TD_SOUND::OscillatorImpl` and possibly `TD_SOUND::EnvelopeImpl` (if the default AR Envelope doens't work for you). The interface for `TD_SOUND::OscillatorImpl` is the same as the old interface to `TD_SOUND::InstrumentImpl`, with one exception: the `note` function is given the current time in note time. As a refresher, the `TD_SOUND::OscillatorImpl` class has a `note` function which takes the frequency to play (in Hertz) and the note-relative time to play it at (in seconds), and it returns the same from -1.0 to 1.0. For every unique note, time will always start at zero. The `TD_SOUND::EnvelopeImpl` has two functions: `release` and `loud`. The `release` function is expected to return the length of the release for the instrument (in seconds). This is expected to be constant and not vary by note played or anything. The `loud` function is given the current note time (in seconds) and the time the note was released at ((in seconds) or -1.0 if the note hasn't been released yet), and it returns the amplitude of the note, from 0.0 to 1.0. Oscillator and Envelope implementations should not have modifiable internal state. Any internal state should be fixed at construction. You then instantiate a `TD_SOUND::Oscillator` or `TD_SOUND::Envelope` class with a shared pointer to an instance of your `TD_SOUND::OscillatorImpl` or `TD_SOUND::EnvelopeImpl` class. The PIMPL idiom is used here to allow `TD_SOUND::Oscillator`, `TD_SOUND::Envelope`, and `TD_SOUND::Instrument` to be handled with value-like semantics, as I feel value-like semantics are easier to understand, and easier to understand programs are more likely to be correct. FINALLY, you construct your custom Instrument by passing in your custom Oscillator and Envelope.

To use your custom instrument with MML music: you will need to call `TD_SOUND::buildVoiceFromString()` passing the custom instrument as the second argument (and don't change the instrument in the string). This builds a `TD_SOUND::Voice` using your custom instrument, and it needs to be put into a `std::vector` and passed to construct a `TD_SOUND::Maestro`. Finally, this is passed to `TD_SOUND::Venue::getInstance().queueMusic()`. It is annoying, but possible.

Data Races
----------

There are two race conditions that I can think of, one of which effects `olc::SOUND` as well:
* If you call `TD_SOUND::Venue::getInstance().clearQueue()` and then immediately queue up a new piece to be played, it may not be played. There's a race between the calling thread and the sound thread, and sometimes the sound thread will lose. There is no race when `TD_SOUND::Venue::getInstance().addMusicCallback()` is used, as the callback is called from the sound thread. The proper way to flush the music queue and start completely new music is to install a callback that will populate the new music and then call `TD_SOUND::Venue::getInstance().clearQueue()` (unless you are already doing everything from the audio thread).
* There is a minute chance of a crash if one queues up a song at the exact instant one song ends. I'm not sure how well a std::list can handle concurrent modification. To my defense, `olc::SOUND` also adds items to a std::list in one thread and removes items from that same list in a different thread.
