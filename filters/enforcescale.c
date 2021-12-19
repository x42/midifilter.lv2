MFD_FILTER(enforcescale)

#ifdef MX_TTF

	mflt:enforcescale
	TTF_DEFAULTDEF("MIDI Enforce Scale", "MIDI Force Scale")
	, TTF_IPORT(0, "channelf", "Filter Channel", 0, 16, 0,
			PORTENUMZ("Any")
			DOC_CHANF)
	, TTF_IPORT(2, "mode", "Mode",  0, 2, 0, \
			lv2:portProperty lv2:integer; lv2:portProperty lv2:enumeration;
			lv2:scalePoint [ rdfs:label "Discard"  ; rdf:value 0 ] ;
			lv2:scalePoint [ rdfs:label "Always down"  ; rdf:value 1 ] ;
			lv2:scalePoint [ rdfs:label "Always up"  ; rdf:value 2 ] ;
			rdfs:comment "Behaviour towards of off-key notes."
			)
	, TTF_IPORT(1, "scale", "Scale",  0, 11, 0,
			lv2:portProperty lv2:integer; lv2:portProperty lv2:enumeration;
			lv2:scalePoint [ rdfs:label "C"  ; rdf:value 0 ] ;
			lv2:scalePoint [ rdfs:label "C#" ; rdf:value 1 ] ;
			lv2:scalePoint [ rdfs:label "D"  ; rdf:value 2 ] ;
			lv2:scalePoint [ rdfs:label "D#" ; rdf:value 3 ] ;
			lv2:scalePoint [ rdfs:label "E"  ; rdf:value 4 ] ;
			lv2:scalePoint [ rdfs:label "F"  ; rdf:value 5 ] ;
			lv2:scalePoint [ rdfs:label "F#" ; rdf:value 6 ] ;
			lv2:scalePoint [ rdfs:label "G"  ; rdf:value 7 ] ;
			lv2:scalePoint [ rdfs:label "G#" ; rdf:value 8 ] ;
			lv2:scalePoint [ rdfs:label "A"  ; rdf:value 9 ] ;
			lv2:scalePoint [ rdfs:label "A#" ; rdf:value 10 ] ;
			lv2:scalePoint [ rdfs:label "B"  ; rdf:value 11 ] ;
			rdfs:comment "Limit note-on/off messages to this scale."
			)
    , TTF_IPORT(3, "scaleMode", "ScaleMode",  0, 6, 0, \
			lv2:portProperty lv2:integer; lv2:portProperty lv2:enumeration;
			lv2:scalePoint [ rdfs:label "Ionian/Major"  ; rdf:value 0 ] ;
			lv2:scalePoint [ rdfs:label "Aeolian/Minor" ; rdf:value 1 ] ;
			lv2:scalePoint [ rdfs:label "Dorian"  ; rdf:value 2 ] ;
			lv2:scalePoint [ rdfs:label "Phrygian"  ; rdf:value 3 ] ;
			lv2:scalePoint [ rdfs:label "Lydian" ; rdf:value 4 ] ;
			lv2:scalePoint [ rdfs:label "Mixolydian"  ; rdf:value 5 ] ;
			lv2:scalePoint [ rdfs:label "Locrian"  ; rdf:value 6 ] ;
			rdfs:comment "Mode of the Scale"
			)
	; rdfs:comment "Filter note-on/off events depending on musical scale. If the key is changed note-off events of are sent for all active off-key notes." ; 
	.

#elif defined MX_CODE
/*
 * Mode Tonic relative
 * scale Interval sequence Example
 * Ionian     I     W–W–H–W–W–W–H     C–D–E–F–G–A–B–C
 * Dorian     II    W–H–W–W–W–H–W     D–E–F–G–A–B–C–D
 * Phrygian   III   H–W–W–W–H–W–W     E–F–G–A–B–C–D–E
 * Lydian     IV    W–W–W–H–W–W–H     F–G–A–B–C–D–E–F
 * Mixolydian V     W–W–H–W–W–H–W     G–A–B–C–D–E–F–G
 * Aeolian    VI    W–H–W–W–H–W–W     A–B–C–D–E–F–G–A
 * Locrian    VII   H–W–W–H–W–W–W     B–C–D–E–F–G–A–B
 *
 */

static int filter_enforcescale_check(int scale,int scalemode, uint8_t key) {
	const short scales[7][12] = {
		{ 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1 },  // Ionian/Major
		{ 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0 },  // Aeolian/Minor
		{ 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0 },  // Dorian
		{ 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0 },  // Phrygian
		{ 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1 },  // Lydian
		{ 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0 },  // Mixolydian
		{ 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0 },  // Locrian

	};
	return scales[scalemode][(key - scale + 12) % 12];
}

static inline void filter_enforcescale_panic(MidiFilter* self, const uint8_t c, const uint32_t tme) {
	int k;
	for (k=0; k < 127; ++k) {
		if (self->memCS[c][k] > 0) {
			uint8_t buf[3];
			buf[0] = MIDI_NOTEOFF | c;
			buf[1] = k;
			buf[2] = 0;
			forge_midimessage(self, tme, buf, 3);
		}
		self->memCI[c][k] = 0; // current key transpose
		self->memCS[c][k] = 0; // count note-on per key
	}
}

static void
filter_midi_enforcescale(MidiFilter* self,
		uint32_t tme,
		const uint8_t* const buffer,
		uint32_t size)
{
	const int chs = midi_limit_chn(floorf(*self->cfg[0]) -1);
	const int scale = RAIL(floorf(*self->cfg[1]), 0, 11);
	const int mode  = RAIL(floorf(*self->cfg[2]), 0, 2);
	const int scalemode  = RAIL(floorf(*self->cfg[3]), 0, 6);

	const uint8_t chn = buffer[0] & 0x0f;
	const uint8_t key = buffer[1] & 0x7f;
	uint8_t mst = buffer[0] & 0xf0;

	if (midi_is_panic(buffer, size)) {
		filter_enforcescale_panic(self, chn, tme);
	}

	if (size != 3
			|| !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF || mst == MIDI_POLYKEYPRESSURE)
			|| !(floorf(*self->cfg[0]) == 0 || chs == chn)
			)
	{
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	int transp = 0;

	if (!filter_enforcescale_check(scale, scalemode, key)) {
		switch (mode) {
			case 1:
					transp = -1;
				break;
			case 2:
					transp = +1;
				break;
			case 0: /* discard */
			default:
				return;
		}
	}

	if (!midi_valid(key + transp)) {
		return;
	}

	if (!filter_enforcescale_check(scale, scalemode,key + transp)) {
		return;
	}

	int note;
	uint8_t buf[3];
	memcpy(buf, buffer, 3);

	switch (mst) {
		case MIDI_NOTEON:
			note = key + transp;
			if (midi_valid(note)) {
				buf[1] = note;
				self->memCS[chn][note]++;
				if (self->memCS[chn][note] == 1)
					forge_midimessage(self, tme, buf, size);
			}
			self->memCI[chn][key] = transp;
			break;
		case MIDI_NOTEOFF:
			note = key + self->memCI[chn][key];
			if (midi_valid(note)) {
				buf[1] = note;
				if (self->memCS[chn][note] > 0) {
					self->memCS[chn][note]--;
					if (self->memCS[chn][note] == 0)
						forge_midimessage(self, tme, buf, size);
					self->memCI[chn][key] = 0;
				}
			}
			break;
		case MIDI_POLYKEYPRESSURE:
			note = key + transp;
			if (midi_valid(note)) {
				buf[1] = note;
				forge_midimessage(self, tme, buf, size);
			}
			break;
	}
}

static void filter_preproc_enforcescale(MidiFilter* self) {
	if (floorf(self->lcfg[1]) == floorf(*self->cfg[1])) return;
	const int scale = RAIL(floorf(*self->cfg[1]), 0, 11);
	const int scalemode = RAIL(floorf(*self->cfg[3]), 0, 6);

	int c,k;
	uint8_t buf[3];
	buf[2] = 0;
	for (c=0; c < 16; ++c) {
		for (k=0; k < 127; ++k) {
			if (self->memCS[c][k] ==0) continue;

			if (filter_enforcescale_check(scale, scalemode, k)) {
				self->memCI[c][k] = 0;
				continue;
			}

			buf[0] = MIDI_NOTEOFF | c;
			buf[1] = midi_limit_val(k);
			buf[2] = 0;
			forge_midimessage(self, 0, buf, 3);
			self->memCS[c][k] = 0;
			self->memCI[c][k] = 0;
		}
	}
}

static void filter_init_enforcescale(MidiFilter* self) {
	int c,k;
	for (c=0; c < 16; ++c) for (k=0; k < 127; ++k) {
		self->memCI[c][k] = 0; // current key transpose
		self->memCS[c][k] = 0; // count note-on per key
	}
	self->preproc_fn = filter_preproc_enforcescale;
}

#endif
