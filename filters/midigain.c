MFD_FILTER(midigain)

#ifdef MX_TTF

	mflt:midigain
	TTF_DEFAULTDEF("MIDI Gain")
	, TTF_IPORT(0, "channel", "Channel",  0.0, 16.0,  0.0, \
			lv2:portProperty lv2:integer; lv2:scalePoint [ rdfs:label "all channels" ; rdf:value 0.0 ])
	, TTF_IPORTFLOAT(1, "gain", "Midi Velocity Gain",  0.0, 4.0,  1.0)
	.

#elif defined MX_CODE

static void filter_init_midigain(MidiFilter* self) { }

static void
filter_midi_midigain(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const int chs = floor(*self->cfg[0]);
	const int chn = buffer[0] & 0x0f;
	const int msg = buffer[0] & 0xf0;

	if (size == 3
			&& (msg == 0x90 || msg == 0x80) // note on, off
			&& (chs == 0 || chs == chn)
		 )
	{
		uint8_t buf[3];
		const float gain = *(self->cfg[1]);
		buf[0] = buffer[0];
		buf[1] = buffer[1];
		buf[2] = midi_limit(rintf(buffer[2] * gain));
		forge_midimessage(self, tme, buf, size);
	} else {
		forge_midimessage(self, tme, buffer, size);
	}
}

#endif
