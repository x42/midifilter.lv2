MFD_FILTER(passthru)

#ifdef MX_TTF

	mflt:passthru
	TTF_DEFAULTDEF("MIDI Thru")
	.

#elif defined MX_CODE

void
filter_midi_passthru(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	forge_midimessage(self, tme, buffer, size);
}

#endif
