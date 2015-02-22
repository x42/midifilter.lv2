midifilter.LV2 Plugins
======================

A collection of 25+ MIDI data filters in LV2 plugin format.
for OSX 10.5 or later, i386 and x64_64 architectures.

The filters follow the basic rules:

 * One MIDI input, one MIDI output
 * No custom GUI, control inputs only
 * Every control can be automated, the plugins handle dynamic parameter changes
 * All plugins report their latency to the host (for most of them it is zero)
 * DRY (Don't Repeat Yourself principle) - simple filters that can be combined in a network


Plugin Installation
-------------------

Place the contents of the meters.lv2 folder to
  $HOME/Library/Audio/Plug-Ins/LV2
or
  /Library/Audio/Plug-Ins/LV2/

see also:
http://x42-plugins.com/
https://x42.github.io/midifilter.lv2/
