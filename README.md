midifilter.lv2
==============

LV2 plugins to filter MIDI events.

So far 27 MIDI event filters have been implemented:

*   CC2Note -- translate control-commands to note-on/off messages
*   Channel Filter -- discard messages per channel
*   Channel Map -- map any MIDI-channel to another MIDI-channel
*   Enforce Scale -- force midi notes on given musical scale
*   Eventblocker -- notch style message filter. Suppress specific messages
*   Keyrange -- discard notes-on/off events outside a given range
*   Keysplit -- change midi-channel number depending on note (and optionally transpose)
*   MapCC -- change one control message into another
*   Mapscale -- flexible 12-tone map
*   Chord -- harmonizer - create chords from a single note in a given musical scale
*   Delay -- delay MIDI events with optional randomization
*   Dup -- unisono - duplicate MIDI events from one channel to another
*   Strum -- arpeggio effect intended to simulate strumming a stringed instrument (e.g. guitar)
*   Transpose -- chromatic transpose MIDI notes
*   Legato -- hold a note until the next note arrives
*   NoSensing -- strip MIDI Active-Sensing events
*   NoDup -- MIDI duplicate blocker. Filter out overlapping note on/off and duplicate messages
*   Note2CC -- convert MIDI note-on messages to control change messages
*   NoteToggle -- toggle notes: play a note to turn it on, play it again to turn it off
*   nTabDelay -- repeat notes N times (incl tempo-ramps -- eurotechno hell yeah)
*   Passthru -- no operation, just pass the MIDI event through (example plugin)
*   Quantize -- live midi event quantization
*   Velocity Randomizer -- randomly change velocity of note-on events
*   ScaleCC -- modify the value (data-byte) of a MIDI control change message
*   Sostenuto -- delay note-off messages, emulate a piano sostenuto pedal
*   Velocity Range -- filter MIDI note events according to velocity
*   Velocity Scale -- modify note velocity by constant factor and offset

Install
-------

Compiling the plugins requires LV2 SDK, gnu-make and a c-compiler.

```bash
  git clone git://github.com/x42/midifilter.lv2.git
  cd midifilter.lv2
  make
  sudo make install PREFIX=/usr
```

Usage
-----

Currently, midifilter is known to work in
[Jalv](http://drobilla.net/software/jalv/) 1.4.6.

Use one of those :
```bash
jalv.gtk http://gareus.org/oss/lv2/midifilter#cctonote
jalv.gtk http://gareus.org/oss/lv2/midifilter#channelfilter
jalv.gtk http://gareus.org/oss/lv2/midifilter#channelmap
jalv.gtk http://gareus.org/oss/lv2/midifilter#enforcescale
jalv.gtk http://gareus.org/oss/lv2/midifilter#eventblocker
jalv.gtk http://gareus.org/oss/lv2/midifilter#keyrange
jalv.gtk http://gareus.org/oss/lv2/midifilter#keysplit
jalv.gtk http://gareus.org/oss/lv2/midifilter#mapcc
jalv.gtk http://gareus.org/oss/lv2/midifilter#mapkeyscale
jalv.gtk http://gareus.org/oss/lv2/midifilter#midichord
jalv.gtk http://gareus.org/oss/lv2/midifilter#mididelay
jalv.gtk http://gareus.org/oss/lv2/midifilter#mididup
jalv.gtk http://gareus.org/oss/lv2/midifilter#midistrum
jalv.gtk http://gareus.org/oss/lv2/midifilter#miditranspose
jalv.gtk http://gareus.org/oss/lv2/midifilter#monolegato
jalv.gtk http://gareus.org/oss/lv2/midifilter#noactivesensing
jalv.gtk http://gareus.org/oss/lv2/midifilter#nodup
jalv.gtk http://gareus.org/oss/lv2/midifilter#notetocc
jalv.gtk http://gareus.org/oss/lv2/midifilter#notetoggle
jalv.gtk http://gareus.org/oss/lv2/midifilter#ntapdelay
jalv.gtk http://gareus.org/oss/lv2/midifilter#passthru
jalv.gtk http://gareus.org/oss/lv2/midifilter#quantize
jalv.gtk http://gareus.org/oss/lv2/midifilter#randvelocity
jalv.gtk http://gareus.org/oss/lv2/midifilter#scalecc
jalv.gtk http://gareus.org/oss/lv2/midifilter#sostenuto
jalv.gtk http://gareus.org/oss/lv2/midifilter#velocityrange
jalv.gtk http://gareus.org/oss/lv2/midifilter#velocityscale
```
