midifilter.lv2
==============

LV2 plugins to filter MIDI events.

So far 13 MIDI event filters have been implemented:

*   Channel Map -- map any MIDI-channel to another MIDI-channel
*   Channel Filter -- discard messages per Channel
*   Delay -- delay MIDI events with optional radomization
*   Enforce Scale -- force midi notes on given musical scale
*   Keysplit -- split note on/off/pressure messages to different channels & optionally transpose
*   NoSensing -- strip MIDI Active-Sensing events
*   Sostenuto -- delay note-off messages, emulate a piano sostenuto pedal
*   Transpose -- chromatic transpose MIDI notes
*   Passthru -- no operation, just pass the MIDI event through (example)
*   Unison  -- duplicate events from one MIDI channel to another
*   Velocity -- modify note velocity by constant factor
*   Velocity Randomizer -- randomly change velocity of note-on events
*   Velocity Randomizer (Normal) -- change velocity of note-on events according to normalized random distribution


Install
-------

Compiling the plugins requires LV2 SDK, gnu-make and a c-compiler.

```bash
  git clone git://github.com/x42/midifilter.lv2.git
  cd midifilter.lv2
  make
  sudo make install PREFIX=/usr
```
