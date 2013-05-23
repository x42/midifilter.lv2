MFD_FILTER(midigain)

#ifdef MX_TTF

	mflt:midigain
	TTF_DEFAULTDEF("MIDI Gain")
	, TTF_FLOATPORT( 2, "gain", "Gain",  0.0, 4.0,  1.0)
	.

#elif defined MX_CODE

void
filter_midi_midigain(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	if (size == 3 && (
				((buffer[0] & 0xf0) != 0x90) // Note on
			||
				((buffer[0] & 0xf0) != 0x80) // Note off
			)
		 )
	{
		uint8_t buf[3];
		const float gain = *(self->cfg[0]);
		buf[0] = buffer[0];
		buf[1] = buffer[1];
		buf[2] = midi_limit(rintf(buffer[2] * gain));
		forge_midimessage(self, tme, buf, size);
	} else {
		forge_midimessage(self, tme, buffer, size);
	}
}

#endif
