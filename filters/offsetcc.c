MFD_FILTER(offsetcc)

#ifdef MX_TTF

	mflt:offsetcc
	TTF_DEFAULTDEF("MIDI CC Offset Parameter", "MIDI CC Offset Parameter")
	, TTF_IPORT(0, "channelf", "Filter Channel", 0, 16, 0,
			PORTENUMZ("Any")
			DOC_CHANF)
	, TTF_IPORT(1, "ccoff", "CC Offset", -127, 127, 0,
			lv2:portProperty lv2:integer;
			rdfs:comment "Offset to add to the CC control parameter"
			)
	; rdfs:comment "Change one control message into another"
	.

#elif defined MX_CODE

void filter_init_offsetcc(MidiFilter* self) { }

void
filter_midi_offsetcc(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const uint8_t chs = midi_limit_chn(floorf(*self->cfg[0]) -1);
	const uint8_t chn = buffer[0] & 0x0f;
	const uint8_t mst = buffer[0] & 0xf0;

	if (size != 3
			|| !(mst == MIDI_CONTROLCHANGE)
			|| !(floorf(*self->cfg[0]) == 0 || chs == chn)
		 )
	{
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	int cc  = buffer[1] & 0x7f;
	int off = floor(*self->cfg[1]);

	if (off < -127) {
		off = -127;
	} else if (off > 127) {
		off = 127;
	}

	cc += off;
	if (cc < 0) {
		cc += 127;
	}

	const uint8_t ccout = cc & 0x7f;

	uint8_t buf[3];
	buf[0] = buffer[0];
	buf[1] = ccout;
	buf[2] = buffer[2];

	forge_midimessage(self, tme, buf, 3);
}

#endif
