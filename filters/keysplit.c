MFD_FILTER(keysplit)

#ifdef MX_TTF

	mflt:keysplit
	TTF_DEFAULTDEF("MIDI Keysplit")
	, TTF_IPORT(0, "channelf", "Filter Channel",  0.0, 16.0,  0.0, PORTENUMZ("Any"))
	, TTF_IPORT(1, "split", "Splitpoint",  0.0, 127.0,  48.0, lv2:portProperty lv2:integer; units:unit units:midiNote)
	, TTF_IPORT(2, "channel0", "Channel Lower",  1.0, 16.0,  1.0, PORTENUM16)
	, TTF_IPORT(3, "transp0", "Transpose Lower",  -48.0, 48.0,  0.0, lv2:portProperty lv2:integer; units:unit units:midiNote)
	, TTF_IPORT(4, "channel1", "Channel Upper",  1.0, 16.0,  2.0, PORTENUM16)
	, TTF_IPORT(5, "transp1", "Transpose Upper",  -48.0, 48.0,  0.0, lv2:portProperty lv2:integer; units:unit units:midiNote)
	.

#elif defined MX_CODE

static void
filter_init_keysplit(MidiFilter* self)
{
	int i;
	for (i=0; i < 127; ++i) {
		self->memI[i] = -1000;
	}
}

static void
filter_midi_keysplit(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const int chs = midi_limit_chn(floor(*self->cfg[0]) -1);
	const uint8_t chn = buffer[0] & 0x0f;
	uint8_t mst = buffer[0] & 0xf0;

	if (size != 3
			|| !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF || mst == MIDI_POLYKEYPRESSURE)
			|| !(floor(*self->cfg[0]) == 0 || chs == chn)
		 )
	{
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	const uint8_t key = buffer[1] & 0x7f;
	const uint8_t vel = buffer[2] & 0x7f;

	if (mst == MIDI_NOTEON && vel ==0 ) {
		mst = MIDI_NOTEOFF;
	}

	const int split = floor(*self->cfg[1]);
	const int ch0 = midi_limit_chn(floor(*self->cfg[2]) -1);
	const int ch1 = midi_limit_chn(floor(*self->cfg[4]) -1);
	const int transp0 = rint(*self->cfg[3]);
	const int transp1 = rint(*self->cfg[5]);

	uint8_t buf[3];
	buf[2] = buffer[2];

	switch (mst) {
		case MIDI_NOTEON:
			if (key < split) {
				buf[0] = mst | ch0;
				buf[1] = midi_limit_val(key + transp0);
				self->memI[key] = transp0;
			} else {
				buf[0] = mst | ch1;
				buf[1] = midi_limit_val(key + transp1);
				self->memI[key] = transp1;
			}
			break;
		case MIDI_NOTEOFF:
			if (key < split) {
				buf[0] = mst | ch0;
				buf[1] = midi_limit_val(key + self->memI[key]);
				self->memI[key] = -1000;
			} else {
				buf[0] = mst | ch1;
				buf[1] = midi_limit_val(key + self->memI[key]);
				self->memI[key] = -1000;
			}
			break;
		case MIDI_POLYKEYPRESSURE:
			if (key < split) {
				buf[0] = mst | ch0;
				buf[1] = midi_limit_val(key + transp0);
			} else {
				buf[0] = mst | ch1;
				buf[1] = midi_limit_val(key + transp1);
			}
			break;
	}
	forge_midimessage(self, tme, buf, size);
}

#endif
