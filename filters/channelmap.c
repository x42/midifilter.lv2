MFD_FILTER(channelmap)

#ifdef MX_TTF

	mflt:channelmap
	TTF_DEFAULTDEF("MIDI Channel Map")
	, TTF_IPORTINT( 0, "chn1",  "Channel  1 to:", 1.0, 16.0,  1.0)
	, TTF_IPORTINT( 1, "chn2",  "Channel  2 to:", 1.0, 16.0,  2.0)
	, TTF_IPORTINT( 2, "chn3",  "Channel  3 to:", 1.0, 16.0,  3.0)
	, TTF_IPORTINT( 3, "chn4",  "Channel  4 to:", 1.0, 16.0,  4.0)
	, TTF_IPORTINT( 4, "chn5",  "Channel  5 to:", 1.0, 16.0,  5.0)
	, TTF_IPORTINT( 5, "chn6",  "Channel  6 to:", 1.0, 16.0,  6.0)
	, TTF_IPORTINT( 6, "chn7",  "Channel  7 to:", 1.0, 16.0,  7.0)
	, TTF_IPORTINT( 7, "chn8",  "Channel  8 to:", 1.0, 16.0,  8.0)
	, TTF_IPORTINT( 8, "chn9",  "Channel  9 to:", 1.0, 16.0,  9.0)
	, TTF_IPORTINT( 9, "chn10", "Channel 10 to:", 1.0, 16.0, 10.0)
	, TTF_IPORTINT(10, "chn11", "Channel 11 to:", 1.0, 16.0, 11.0)
	, TTF_IPORTINT(11, "chn12", "Channel 12 to:", 1.0, 16.0, 12.0)
	, TTF_IPORTINT(12, "chn13", "Channel 13 to:", 1.0, 16.0, 13.0)
	, TTF_IPORTINT(13, "chn14", "Channel 14 to:", 1.0, 16.0, 14.0)
	, TTF_IPORTINT(14, "chn15", "Channel 15 to:", 1.0, 16.0, 15.0)
	, TTF_IPORTINT(15, "chn16", "Channel 16 to:", 1.0, 16.0, 16.0)
	.

#elif defined MX_CODE

void filter_init_channelmap(MidiFilter* self) { }

void
filter_midi_channelmap(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	uint8_t buf[3];
	if (size > 3) {
		forge_midimessage(self, tme, buffer, size);
		return;
	} else {
		memcpy(buf, buffer, size);
	}

	int chn = buffer[0]&0x0f;
	switch (buffer[0] & 0xf0) {
		case 0x80: // note off
		case 0x90: // note on
		case 0xA0: // Polyphonic Key Pressure (Aftertouch)
		case 0xB0: // control change
		case 0xC0: // program change
		case 0xD0: // Channel Pressure (After-touch)
		case 0xE0: // pitch wheel
			chn = (int) floor(-1 + *(self->cfg[chn]));
			if (chn < 0)    chn = 0;
			if (chn > 0xf)  chn = 0xf;
			buf[0] = (buffer[0] & 0xf0) | chn;
			break;
		default:
			break;
	}
	forge_midimessage(self, tme, buf, size);
}

#endif
