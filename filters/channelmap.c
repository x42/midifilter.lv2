MFD_FILTER(channelmap)

#ifdef MX_TTF

	mflt:channelmap
	TTF_DEFAULTDEF("MIDI Channel Map")
	, TTF_INTPORT( 2, "chn1", "Channel  1",  1.0, 16.0,  1.0)
	, TTF_INTPORT( 3, "chn2", "Channel  2",  1.0, 16.0,  2.0)
	, TTF_INTPORT( 4, "chn3", "Channel  3",  1.0, 16.0,  3.0)
	, TTF_INTPORT( 5, "chn4", "Channel  4",  1.0, 16.0,  4.0)
	, TTF_INTPORT( 6, "chn5", "Channel  5",  1.0, 16.0,  5.0)
	, TTF_INTPORT( 7, "chn6", "Channel  6",  1.0, 16.0,  6.0)
	, TTF_INTPORT( 8, "chn7", "Channel  7",  1.0, 16.0,  7.0)
	, TTF_INTPORT( 9, "chn8", "Channel  8",  1.0, 16.0,  8.0)
	, TTF_INTPORT(10, "chn9", "Channel  9",  1.0, 16.0,  9.0)
	, TTF_INTPORT(11, "chn10", "Channel 10", 1.0, 16.0, 10.0)
	, TTF_INTPORT(12, "chn11", "Channel 11", 1.0, 16.0, 11.0)
	, TTF_INTPORT(13, "chn12", "Channel 12", 1.0, 16.0, 12.0)
	, TTF_INTPORT(14, "chn13", "Channel 13", 1.0, 16.0, 13.0)
	, TTF_INTPORT(15, "chn14", "Channel 14", 1.0, 16.0, 14.0)
	, TTF_INTPORT(16, "chn15", "Channel 15", 1.0, 16.0, 15.0)
	, TTF_INTPORT(17, "chn16", "Channel 16", 1.0, 16.0, 16.0)
	.

#elif defined MX_CODE

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
			if (chn < 0)  chn = 0;
			if (chn > 0xf)  chn = 0xf;
			buf[0] = (buffer[0] & 0xf0) | chn;
			break;
		default:
			break;
	}
	forge_midimessage(self, tme, buf, size);
}

#endif
