MFD_FILTER(notetopgm)

#ifdef MX_TTF

	mflt:notetopgm
	TTF_DEFAULTDEF("MIDI Note to PC", "MIDI Note to PC")
	, TTF_IPORT(0, "channelf", "Filter Channel", 0, 16, 0,
			PORTENUMZ("Any")
			DOC_CHANF)
	, TTF_IPORT(1, "mode", "Mode",  0, 11,  0,
			lv2:portProperty lv2:integer; lv2:portProperty lv2:enumeration;
			lv2:scalePoint [ rdfs:label "Chromatic Map" ; rdf:value 0 ] ;
			lv2:scalePoint [ rdfs:label "C -1" ; rdf:value 1 ] ;
			lv2:scalePoint [ rdfs:label "C 00" ; rdf:value 2 ] ;
			lv2:scalePoint [ rdfs:label "C 01" ; rdf:value 3 ] ;
			lv2:scalePoint [ rdfs:label "C 02" ; rdf:value 4 ] ;
			lv2:scalePoint [ rdfs:label "C 03" ; rdf:value 5 ] ;
			lv2:scalePoint [ rdfs:label "C 04" ; rdf:value 6 ] ;
			lv2:scalePoint [ rdfs:label "C 05" ; rdf:value 7 ] ;
			lv2:scalePoint [ rdfs:label "C 06" ; rdf:value 8 ] ;
			lv2:scalePoint [ rdfs:label "C 07" ; rdf:value 9 ] ;
			lv2:scalePoint [ rdfs:label "C 08" ; rdf:value 10 ] ;
			lv2:scalePoint [ rdfs:label "C 09" ; rdf:value 11 ] ;
			rdfs:comment "Mapping from note to patch-change. This is either chromatic mapping start at the lowest note (C-1), or white-keys (C-major scale) starting at a given C."
			)
	, TTF_IPORT(2, "off",  "Offset",  -64, 64, 0,
			lv2:portProperty lv2:integer;
			rdfs:comment "Offset patch-change number")
	, TTF_IPORT(3, "minvel", "Min Velocity",  0, 127, 1, lv2:portProperty lv2:integer)
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
	const uint8_t vel = buffer[2] & 0x7f;

	const int     mode = RAIL(floorf(*self->cfg[1]), 0, 11);
	const int     off  = RAIL(floorf(*self->cfg[2]),-64, 64);
	const uint8_t mvel = midi_limit_val(floorf(*self->cfg[3]));

	uint8_t pgm = key;

	if (mode == 0) {
		int p = (128 + key + off);
		pgm = p & 0x7f;
	} else {
		/* major scale map */
		static const int scale[] = { 0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6 };
		int root = (mode - 1);
		int octv =  key / 12;
		int note =  key % 12;
		if (scale[note] < 0) {
			return;
		}
		int p = 256 + scale[note] + (octv - root) * 7 + off;
		pgm = p & 0x7f;
	}

	uint8_t buf[2];
	buf[0] = MIDI_PROGRAMCHANGE | chn;
	buf[1] = pgm;

	if ((mst == MIDI_NOTEON && vel >= mvel) || mvel == 0) {
		forge_midimessage(self, tme, buf, 2);
	}
}

#endif
