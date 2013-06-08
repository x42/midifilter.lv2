MFD_FILTER(midichord)

#ifdef MX_TTF

	mflt:midichord
	TTF_DEFAULTDEF("MIDI Chord")
	, TTF_IPORT(0, "channelf", "Filter Channel",  0.0, 16.0,  0.0,
			PORTENUMZ("Any")
			DOC_CHANF)
	, TTF_IPORT(1, "scale", "Scale",  0.0, 11.0,  0.0,
			lv2:portProperty lv2:integer; lv2:portProperty lv2:enumeration;
			lv2:scalePoint [ rdfs:label "C Major"  ; rdf:value 0.0 ] ;
			lv2:scalePoint [ rdfs:label "C# Major" ; rdf:value 1.0 ] ;
			lv2:scalePoint [ rdfs:label "D Major"  ; rdf:value 2.0 ] ;
			lv2:scalePoint [ rdfs:label "D# Major" ; rdf:value 3.0 ] ;
			lv2:scalePoint [ rdfs:label "E Major"  ; rdf:value 4.0 ] ;
			lv2:scalePoint [ rdfs:label "F Major"  ; rdf:value 5.0 ] ;
			lv2:scalePoint [ rdfs:label "F# Major" ; rdf:value 6.0 ] ;
			lv2:scalePoint [ rdfs:label "G Major"  ; rdf:value 7.0 ] ;
			lv2:scalePoint [ rdfs:label "G# Major" ; rdf:value 8.0 ] ;
			lv2:scalePoint [ rdfs:label "A Major"  ; rdf:value 9.0 ] ;
			lv2:scalePoint [ rdfs:label "A# Major" ; rdf:value 10.0 ] ;
			lv2:scalePoint [ rdfs:label "B Major"  ; rdf:value 11.0 ] ;
			rdfs:comment "Scale for the Chords."
			)
	, TTF_IPORTTOGGLE( 2, "c1",  "prime", 1.0)
	, TTF_IPORTTOGGLE( 3, "c3",  "3rd", 1.0)
	, TTF_IPORTTOGGLE( 4, "c5",  "5th", 1.0)
	, TTF_IPORTTOGGLE( 5, "c6",  "6th", 0.0)
	, TTF_IPORTTOGGLE( 6, "c7",  "7th", 0.0)
	, TTF_IPORTTOGGLE( 7, "c8",  "octave", 1.0)
	, TTF_IPORTTOGGLE( 8, "c9",  "9th", 0.0)
	, TTF_IPORTTOGGLE( 9, "c11", "11th", 0.0)
	, TTF_IPORTTOGGLE(10, "c13", "13th", 0.0)
	, TTF_IPORTTOGGLE(11, "_8",  "bass", 0.0)
	; rdfs:comment "" ;
	.

#elif defined MX_CODE

static inline int filter_midichord_isonscale(int base) {
	const short major_scale[12] = {
		1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1
	};
	return major_scale[base];
}

static inline int filter_midichord_halftoneoffset(int base, int interval) {
	const short chord_scale[12][10] = {
	/* 1  3  5  6   7  OC   9  11  13   BS */
		{0, 4, 7, 9, 11, 12, 14, 17, 21, -12 },
		{0, 0, 0, 0,  0,  0,  0,  0,  0,   0 },
		{0, 3, 7, 9, 10, 12, 14, 17, 21, -12 },
		{0, 0, 0, 0,  0,  0,  0,  0,  0,   0 },
		{0, 3, 7, 8, 10, 12, 13, 17, 20, -12 },
		{0, 4, 7, 9, 11, 12, 14, 18, 21, -12 },
		{0, 0, 0, 0,  0,  0,  0,  0,  0,   0 },
		{0, 4, 7, 9, 10, 12, 14, 17, 21, -12 },
		{0, 0, 0, 0,  0,  0,  0,  0,  0,   0 },
		{0, 3, 7, 8, 10, 12, 14, 17, 20, -12 },
		{0, 0, 0, 0,  0,  0,  0,  0,  0,   0 },
		{0, 3, 6, 8, 10, 12, 13, 16, 19, -12 },
	};
	return chord_scale[base][interval];
}

static inline void filter_midichord_noteon(MidiFilter* self, uint32_t tme, uint8_t chn, int note, uint8_t vel) {
	uint8_t buf[3];
	if (!midi_valid(note)) return;
	buf[0] = MIDI_NOTEON | chn;
	buf[1] = note;
	buf[2] = vel;
	self->memCS[chn][note]++;
	if (self->memCS[chn][note] == 1) {
		forge_midimessage(self, tme, buf, 3);
	}
}

static inline void filter_midichord_noteoff(MidiFilter* self, uint32_t tme, uint8_t chn, int note, uint8_t vel) {
	uint8_t buf[3];
	if (!midi_valid(note)) return;
	buf[0] = MIDI_NOTEOFF | chn;
	buf[1] = note;
	buf[2] = vel;
	if (self->memCS[chn][note] > 0) {
		self->memCS[chn][note]--;
		if (self->memCS[chn][note] == 0)
			forge_midimessage(self, tme, buf, 3);
	}
}

static inline void filter_midichord_panic(MidiFilter* self, const uint8_t c, const uint32_t tme) {
	int k;
	for (k=0; k < 127; ++k) {
		if (self->memCS[c][k] > 0) {
			uint8_t buf[3];
			buf[0] = MIDI_NOTEOFF | c;
			buf[1] = k;
			buf[2] = 0;
			forge_midimessage(self, tme, buf, 3);
		}
		self->memCI[c][k] = -1000; // current chord for this key
		self->memCS[c][k] = 0; // count note-on per key
		self->memCM[c][k] = 0; // last known velocity for this key
	}
}

static void
filter_midi_midichord(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	int i;
	const int chs = midi_limit_chn(floor(*self->cfg[0]) -1);
	const int scale = RAIL(floor(*self->cfg[1]), 0, 11);

	int chord = 0;
	for (i=0; i < 10 ; ++i) {
		if ((*self->cfg[i+2]) != 0) chord |= 1<<i;
	}

	const uint8_t chn = buffer[0] & 0x0f;
	uint8_t mst = buffer[0] & 0xf0;

	if (midi_is_panic(buffer, size)) {
		filter_midichord_panic(self, chn, tme);
	}

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
	const int tonika = (key + 12 - scale) % 12;

	if (! filter_midichord_isonscale(tonika)) {
		chord = 1;
	}

	switch (mst) {
		case MIDI_NOTEON:
			self->memCI[chn][key] = chord;
			self->memCM[chn][key] = vel;
			for (i=0; i < 10 ; ++i) {
				if (!(chord & (1<<i))) continue;
				filter_midichord_noteon(self, tme, chn, key + filter_midichord_halftoneoffset(tonika, i), vel);
			}
			break;
		case MIDI_NOTEOFF:
			chord = self->memCI[chn][key];
			for (i=0; i < 10 ; ++i) {
				if (!(chord & (1<<i))) continue;
				filter_midichord_noteoff(self, tme, chn, key + filter_midichord_halftoneoffset(tonika, i), vel);
			}
			self->memCI[chn][key] = -1000;
			self->memCM[chn][key] = 0;
			break;
		case MIDI_POLYKEYPRESSURE:
			for (i=0; i < 10 ; ++i) {
				uint8_t buf[3];
				if (!(chord & (1<<i))) continue;
				int note = key + filter_midichord_halftoneoffset(tonika, i);
				if (midi_valid(note)) {
					buf[0] = buffer[0];
					buf[1] = note;
					buf[2] = buffer[2];
					forge_midimessage(self, tme, buf, size);
				}
			}
			break;
	}
}

static void filter_preproc_midichord(MidiFilter* self) {
	int c,k,i;
	int identical_cfg = 1;
	int newchord = 0;

	for (i=0; i < 10; ++i) {
		if ((*self->cfg[i+2]) != 0) newchord |= 1<<i;
		if (floor(self->lcfg[i+2]) != floor(*self->cfg[i+2])) {
			identical_cfg = 0;
		}
	}
	if (floor(self->lcfg[1]) != floor(*self->cfg[1])) {
			identical_cfg = 0;
	}
	if (identical_cfg) return;

	const int newscale = RAIL(floor(*self->cfg[1]), 0, 11);
	const int oldscale = RAIL(floor(self->lcfg[1]), 0, 11);

	for (c=0; c < 16; ++c) {
		for (k=0; k < 127; ++k) {
			if (self->memCM[c][k] == 0) continue;
			if (self->memCI[c][k] == -1000) continue;

			const uint8_t vel = self->memCM[c][k];
			const int t0 = (k + 12 - oldscale) % 12;
			const int t1 = (k + 12 - newscale) % 12;

			const int oldchord = self->memCI[c][k];
			int chord = newchord;

			if (! filter_midichord_isonscale(t1)) {
				chord = 1;
			}

			for (i=0; i < 10 ; ++i) {

				if ((chord & (1<<i)) == (oldchord & (1<<i))
						&& !(chord & (1<<i))) {
					continue;
				}

				if ((chord & (1<<i)) == (oldchord & (1<<i))
						&& (filter_midichord_halftoneoffset(t0, i) == filter_midichord_halftoneoffset(t1, i))
						&& t0 == t1) {
						continue;
				}

				if (oldchord & (1<<i)) {
					filter_midichord_noteoff(self, 0, c, k + filter_midichord_halftoneoffset(t0, i), 0);
				}
				if (chord & (1<<i)) {
					filter_midichord_noteon(self, 0, c, k + filter_midichord_halftoneoffset(t1, i), vel);
				}
			}
			self->memCI[c][k] = chord;
		}
	}
}

static void filter_init_midichord(MidiFilter* self) {
	int c,k;
	for (c=0; c < 16; ++c) for (k=0; k < 127; ++k) {
		self->memCI[c][k] = -1000; // current chord for this key
		self->memCS[c][k] = 0; // count note-on per key
		self->memCM[c][k] = 0; // last known velocity for this key
	}
	self->preproc_fn = filter_preproc_midichord;
}

#endif
