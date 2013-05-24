midifilter.lv2
==============

LV2 plugins to filter MIDI events.

So far 9 MIDI event filters have been implemented:

*   Channel Map -- map any MIDI-channel to another MIDI-channel
*   Keysplit -- split note on/off/pressure messages to different channels & optionally transpose
*   Unison  -- duplicate events from one MIDI channel to another
*   Velocity -- modify note velocity by constant factor
*   Velocity Randomizer -- change velocity of note-on events
*   Velocity Randomizer (Normal) -- change velocity of note-on events according to normalized random distribution
*   Transpose -- chromatic transpose MIDI notes
*   Delay -- delay MIDI events with optional radomization
*   NoSensing -- strip MIDI Active-Sensing events
*   Passthru -- no operation, just pass the MIDI event through (example)


Install
-------

Compiling the plugins requires LV2 SDK, gnu-make and a c-compiler.

```bash
  git clone git://github.com/x42/midifilter.lv2.git
  cd midifilter.lv2
  make
  sudo make install PREFIX=/usr
```
