midifilter.lv2
==============

LV2 audio plugins to filter MIDI events.

So far 7 MIDI event filters have been implemented:

*   Channel Map -- map any MIDI-channel to another MIDI-channel
*   Keysplit -- split note on/off/pressure to different channels & transpose
*   Unisono  -- duplicate events from one channel to another
*   Gain -- modify note velocity
*   Transpose -- chromatic transpose of midi notes
*   Sensing -- strip midi Active-Sensing events
*   Passtrhu -- no operation, just pass the midi event through (example)


Install
-------

Compiling the plugins requires LV2 SDK, gnu-make and a c-compiler.

```bash
  git clone git://github.com/x42/midifilter.lv2.git
  cd midifilter.lv2
  make
  sudo make install PREFIX=/usr
```
