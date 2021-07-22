MFD_FILTER(notetopgm)

#ifdef MX_TTF

	mflt:notetopgm
	TTF_DEFAULTDEF("MIDI Note to PC", "MIDI Note to PC")
	, TTF_IPORT(0, "channelf", "Filter Channel", 0, 16, 0,
			PORTENUMZ("Any")
			DOC_CHANF)
	, TTF_IPORTTOGGLE(1, "nooff", "Ignore Note Off", 1)
	; rdfs:comment "Convert MIDI note messages to patch/program change messages."
	.

#elif defined MX_CODE

void filter_init_notetopgm(MidiFilter* self) { }

void
filter_midi_notetopgm(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const uint8_t chs = midi_limit_chn(floorf(*self->cfg[0]) -1);
	const uint8_t chn = buffer[0] & 0x0f;
	const uint8_t mst = buffer[0] & 0xf0;

	if (size != 3
			|| !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF)
			|| !(floorf(*self->cfg[0]) == 0 || chs == chn)
		 )
	{
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	const uint8_t key = buffer[1] & 0x7f;

	/* yet unused, consider velocity gate, and key -> pgm mapping */
	//const uint8_t vel = buffer[2] & 0x7f;

	uint8_t buf[2];
	buf[0] = MIDI_PROGRAMCHANGE | chn;
	buf[1] = key;

	// TODO, keep track of note-on? avoid dups?

	if (mst == MIDI_NOTEON || (*(self->cfg[1])) <= 0) {
		forge_midimessage(self, tme, buf, 2);
	}
}

#endif
