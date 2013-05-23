MFD_FILTER(noactivesensing)

#ifdef MX_TTF

	mflt:noactivesensing
	TTF_DEFAULTDEF("MIDI Remove Active Sensing")
	.

#elif defined MX_CODE

void filter_init_noactivesensing(MidiFilter* self) { }

void
filter_midi_noactivesensing(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const int mst = buffer[0] & 0xf0;
	if (mst == MIDI_ACTIVESENSING) return;
	forge_midimessage(self, tme, buffer, size);
}

#endif
