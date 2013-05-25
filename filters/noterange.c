MFD_FILTER(noterange)

#ifdef MX_TTF

	mflt:noterange
	TTF_DEFAULTDEF("MIDI Note-Range Filter")
	, TTF_IPORT(0, "channelf", "Filter Channel",  0.0, 16.0,  0.0, PORTENUMZ("Any"))
	, TTF_IPORT(1, "lower", "Lowest Note",  0.0, 127.0,  0.0, lv2:portProperty lv2:integer; units:unit units:midiNote)
	, TTF_IPORT(2, "upper", "Highest Note",  0.0, 127.0,  127.0, lv2:portProperty lv2:integer; units:unit units:midiNote)
	.

#elif defined MX_CODE

static void filter_init_noterange(MidiFilter* self) { }

static void
filter_midi_noterange(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const uint8_t chs = midi_limit_chn(floor(*self->cfg[0]) -1);
	const uint8_t chn = buffer[0] & 0x0f;
	const uint8_t mst = buffer[0] & 0xf0;

	if (size == 3
			&& (mst == MIDI_NOTEON || mst == MIDI_NOTEOFF)
			&& (floor(*self->cfg[0]) == 0 || chs == chn)
		 )
	{
		const uint8_t low = midi_limit_val(floor(*self->cfg[1]));
		const uint8_t upp = midi_limit_val(floor(*self->cfg[2]));
		const uint8_t key = buffer[1] & 0x7f;
		if (key >= low && key <= upp) {
			forge_midimessage(self, tme, buffer, size);
		}
	} else {
		forge_midimessage(self, tme, buffer, size);
	}
}

#endif
