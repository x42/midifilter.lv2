MFD_FILTER(enforcescale)

#ifdef MX_TTF

	mflt:enforcescale
	TTF_DEFAULTDEF("MIDI Enforce Scale")
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
			rdfs:comment "Limit note-on/off messages to this scale."
			)
	, TTF_IPORT(2, "mode", "Mode",  0.0, 2.0,  0.0, \
			lv2:portProperty lv2:integer; lv2:portProperty lv2:enumeration;
			lv2:scalePoint [ rdfs:label "Discard"  ; rdf:value 0.0 ] ;
			lv2:scalePoint [ rdfs:label "Always down"  ; rdf:value 1.0 ] ;
			lv2:scalePoint [ rdfs:label "Always up"  ; rdf:value 2.0 ] ;
			rdfs:comment "Behaviour towards of off-key notes."
			)
	; rdfs:comment "Filter note-on/off events depending on musical scale. If the key is changed note-off events of are sent for all active off-key notes." ; 
	.

#elif defined MX_CODE


static int filter_enforcescale_check(int scale, uint8_t key) {
	const short major_scale[12] = {
		1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1
	};
	return major_scale[(key - scale + 12) % 12];
}

static inline void filter_enforcescale_panic(MidiFilter* self, uint8_t c, uint32_t tme) {
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
	const int chs = midi_limit_chn(floor(*self->cfg[0]) -1);
	const int scale = RAIL(floor(*self->cfg[1]), 0, 11);
	const int mode  = RAIL(floor(*self->cfg[2]), 0, 2);

	const uint8_t chn = buffer[0] & 0x0f;
	const uint8_t key = buffer[1] & 0x7f;
	uint8_t mst = buffer[0] & 0xf0;

	if (midi_is_panic(buffer, size)) {
		filter_enforcescale_panic(self, chn, tme);
	}

	if (size != 3
			|| !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF || mst == MIDI_POLYKEYPRESSURE)
			|| !(floor(*self->cfg[0]) == 0 || chs == chn)
			)
	{
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	int transp = 0;

	if (!filter_enforcescale_check(scale, key)) {
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

	if (!filter_enforcescale_check(scale, key + transp)) {
		return;
	}

	int note;
	uint8_t buf[3];

	buf[0] = buffer[0];
	buf[1] = buffer[1];
	buf[2] = buffer[2];

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
	if (floor(self->lcfg[1]) == floor(*self->cfg[1])) return;
	const int scale = RAIL(floor(*self->cfg[1]), 0, 11);

	int c,k;
	uint8_t buf[3];
	buf[2] = 0;
	for (c=0; c < 16; ++c) {
		for (k=0; k < 127; ++k) {
			if (self->memCS[c][k] ==0) continue;

			if (filter_enforcescale_check(scale, k)) {
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
