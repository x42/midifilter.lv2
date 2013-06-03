MFD_FILTER(midistrum)

#ifdef MX_TTF

	mflt:midistrum
	TTF_DEF("MIDI Strum", ; atom:supports time:Position)
	, TTF_IPORT( 0, "bpmsrc",  "BPM source", 0.0, 1.0,  1.0,
			lv2:scalePoint [ rdfs:label "Control Port" ; rdf:value 0.0 ] ;
			lv2:scalePoint [ rdfs:label "Plugin Host (if available)" ; rdf:value 1.0 ] ;
			lv2:portProperty lv2:integer; lv2:portProperty lv2:enumeration;
			)
	, TTF_IPORT(1, "bpm",  "BPM", 1.0, 280.0,  120.0, units:unit units:bpm;
			rdfs:comment "base unit for the time (unless host provides BPM)")
	, TTF_IPORT( 2, "mode",  "Strum Mode", 0.0, 3.0,  2.0,
			lv2:scalePoint [ rdfs:label "Always Down (low notes first)" ; rdf:value 0.0 ] ;
			lv2:scalePoint [ rdfs:label "Always Up (high notes first)" ; rdf:value 1.0 ] ;
			lv2:scalePoint [ rdfs:label "Alternate" ; rdf:value 2.0 ] ;
			lv2:scalePoint [ rdfs:label "Up/Down Beat" ; rdf:value 3.0 ] ;
			lv2:portProperty lv2:integer; lv2:portProperty lv2:enumeration;
			)
	, TTF_IPORT(3, "collect", "Note Collect Timeout [ms]", 0.0, 300,  15,
			rdfs:comment "Time to wait for chord to be 'complete'. Keys pressed withing given timeframe will be combined into one chord.")
	, TTF_IPORT(4, "duration", "Strum Duration in Beats", 0.0, 4.0, .25,
			lv2:scalePoint [ rdfs:label "Immediate" ; rdf:value 0.0 ] ;
			lv2:scalePoint [ rdfs:label "32th" ; rdf:value 0.125 ] ;
			lv2:scalePoint [ rdfs:label "16th" ; rdf:value 0.25 ] ;
			lv2:scalePoint [ rdfs:label "Eigth" ; rdf:value 0.5 ] ;
			lv2:scalePoint [ rdfs:label "Quarter" ; rdf:value 1.0 ] ;
			lv2:scalePoint [ rdfs:label "Half Note" ; rdf:value 2.0 ] ;
			lv2:scalePoint [ rdfs:label "Whole Note" ; rdf:value 4.0 ] ;
			rdfs:comment "delay length in base-unit")
	/*
	, TTF_IPORT(5, "randspeed", "Randomize Strum Speed", 0.0, 1.0,  0.0,
			rdfs:comment "duration randomization factor")
	, TTF_IPORT(6, "adjvelocity", "Adjust Velocity", -1.0, 1.0,  0.0,
			rdfs:comment "")
	, TTF_IPORT(7, "randvelocity", "Randomize Velocity", 0.0, 1.0,  0.0,
			rdfs:comment "velodity randomization factor")
	*/
	; rdfs:comment ""
	.

#elif defined MX_CODE

#define MAX_STRUM_CHORDNOTES (12)

static void
filter_cleanup_midistrum(MidiFilter* self)
{
	free(self->memQ);
	free(self->memS);
}

static void
filter_midistrum_process(MidiFilter* self, int tme)
{
	int i;
	const int max_collect = 1 + rintf(self->samplerate * (*self->cfg[3]) / 1000.0);
	if (self->memI[5] == 0) return; // no notes collected

	if (self->memI[4] + max_collect > self->memI[3] + tme) { // TODO handle time overflow
		/* collection time not over */
		if (self->memI[5] < MAX_STRUM_CHORDNOTES) {
			/* buffer is not full, either */
			return;
		}
	}

	// TODO  check that all fit in buffer
	// if ((self->memI[2] + 1) % self->memI[0] == self->memI[1]) { return; }

	float bpm = (*self->cfg[1]);
	if (*self->cfg[0] && (self->available_info & NFO_BPM)) {
		bpm = self->bpm;
	}
	if (bpm <= 0) bpm = 60;
	const int strum_time = floor(self->samplerate * (*self->cfg[4]) * 60.0 / bpm);

	int dir = 0; // 0: down (low notes first), 1: up (high-notes first)
	switch ((int) floor(*self->cfg[2])) {
		case 0: // always down
			break;
		case 1: // always up
			dir = 1;
			break;
		case 2: // alternate
			dir = (self->memI[6]) ? 1 : 0;
			break;
		case 3: // depending on beat.. 1,2,3,4 down ;; 1+,2+,3+,4+ up
			// compensate for latency, round off inacurracies on beat boundaries.
			if ((self->available_info & NFO_BEAT)) {
				const double samples_per_beat = 60.0 / self->bpm * self->samplerate;
				if (ROUND_PARTIAL_BEATS(self->beat_beats + ((tme - max_collect) / samples_per_beat), 12.0) >= 0.5) {
					dir = 1;
				}
			}
			break;
	}
	self->memI[6] = !dir;

	int reltime = self->memI[4] + max_collect - self->memI[3];
	int tdiff = strum_time / self->memI[5];

	/* sort notes by strum direction.. */
	for (i=0; i < self->memI[5]; ++i) {
		int nextup = -1;
		int ii;
		for (ii=0; ii < self->memI[5]; ++ii) {
			if (self->memS[ii].size == 0) continue;
			if (nextup < 0) { nextup = ii; continue;}
			if (dir) {
				if (self->memS[nextup].buf[1] < self->memS[ii].buf[1] ) nextup = ii;
			} else {
				if (self->memS[nextup].buf[1] > self->memS[ii].buf[1] ) nextup = ii;
			}
		}

		// TODO randomness

		MidiEventQueue *qm = &(self->memQ[self->memI[2]]);
		memcpy(qm->buf, self->memS[nextup].buf, self->memS[nextup].size);
		qm->size = self->memS[nextup].size;
		qm->reltime = reltime + tdiff * i;
		self->memI[2] = (self->memI[2] + 1) % self->memI[0];
		self->memS[nextup].size = 0; // mark as processed
	}

	self->memI[5] = 0;
}

static inline void filter_midistrum_panic(MidiFilter* self, uint32_t tme) {
	int i,c,k;
	const int max_delay = self->memI[0];
	for (i=0; i < max_delay; ++i) {
		self->memQ[i].size = 0;
	}
	self->memI[1] = 0; // read-pointer
	self->memI[2] = 0; // write-pointer

	self->memI[4] = 0; // collection stattime
	self->memI[5] = 0; // collected notes
	self->memI[6] = 0; // stroke direction

	for (c=0; c < 16; ++c) for (k=0; k < 127; ++k) {
		if (self->memCS[c][k]) {
			uint8_t buf[3];
			buf[0] = MIDI_NOTEOFF | c;
			buf[1] = k; buf[2] = 0;
			forge_midimessage(self, tme, buf, 3);
		}
		self->memCS[c][k] = 0; // count note-on per key
	}
}

void
filter_midi_midistrum(MidiFilter* self,
		const uint32_t tme,
		const uint8_t* const buffer,
		const uint32_t size)
{
	uint8_t mst = buffer[0] & 0xf0;

	if (size > 3) {
		forge_midimessage(self, tme, buffer, size);
		return;
	}

	if (size == 3
			&& mst == MIDI_CONTROLCHANGE
			&& (buffer[1]&0x7f) == 123
			&& (buffer[2]&0x7f) == 0)
	{
		filter_midistrum_panic(self, tme);
	}

	if (size != 3 || !(mst == MIDI_NOTEON || mst == MIDI_NOTEOFF)) {
		if ((self->memI[2] + 1) % self->memI[0] == self->memI[1]) {
			return; // queue full
		}
		MidiEventQueue *qm = &(self->memQ[self->memI[2]]);
		memcpy(qm->buf, buffer, size);
		qm->size = size;
		qm->reltime = tme;
		self->memI[2] = (self->memI[2] + 1) % self->memI[0];
		return;
	}

	float bpm = (*self->cfg[1]);
	if (*self->cfg[0] && (self->available_info & NFO_BPM)) {
		bpm = self->bpm;
	}
	if (bpm <= 0) bpm = 60;

	const int strum_time = floor(self->samplerate * (*self->cfg[4]) * 60.0 / bpm);
	const int max_collect = 1 + rintf(self->samplerate * (*self->cfg[3]) / 1000.0);

	const uint8_t key = buffer[1] & 0x7f;
	const uint8_t vel = buffer[2] & 0x7f;

	if (mst == MIDI_NOTEON && vel ==0 ) {
		mst = MIDI_NOTEOFF;
	}

	// check if we're overdue -> process collected notes, reset collection mode
	filter_midistrum_process(self, tme);

	/* add note-ons to collection */
	if (mst == MIDI_NOTEON) {
		int i;
		if (self->memI[5] == 0) {
			self->memI[4] = tme + self->memI[3];
		}

		// check if note-on for this key is already queued -> skip
		for (i=0; i < self->memI[5]; ++i) {
			if (self->memS[i].size == 3 && self->memS[i].buf[2] == key) {
				// TODO mark -> no dup note-off
				return;
			}
		}

		MidiEventQueue *qm = &(self->memS[self->memI[5]]);
		memcpy(qm->buf, buffer, size);
		qm->size = size;
		qm->reltime = tme + self->memI[3]; // TODO time-wrap
		self->memI[5]++;
#if 0
		printf("note on #%d! %d / %d || %d %d\n", 
				self->memI[5],
				tme + self->memI[3], self->memI[4],
				tme + self->memI[3] - self->memI[4], max_collect);
#endif
	}

	/* delay note-off by max-latency (= collection-time + strum-time) */
	else if (mst == MIDI_NOTEOFF) {
		int delay = max_collect + strum_time;
		if (self->memI[5] > 0) {
			// TODO filter out ignored dups from above
			delay -= tme + self->memI[3] - self->memI[4];
		}

		MidiEventQueue *qm = &(self->memQ[self->memI[2]]);
		memcpy(qm->buf, buffer, size);
		qm->size = size;
		qm->reltime = tme + delay;
		self->memI[2] = (self->memI[2] + 1) % self->memI[0];
	}
}

static void
filter_preproc_midistrum(MidiFilter* self)
{
	self->latency = 1 + rint(self->samplerate * (*self->cfg[3]) / 1000.0);
}

static void
filter_postproc_midistrum(MidiFilter* self)
{
	int i;
	const int max_delay = self->memI[0];
	const int roff = self->memI[1];
	const int woff = self->memI[2];
	const uint32_t n_samples = self->n_samples;
	int skipped = 0;

	filter_midistrum_process(self, n_samples);

	for (i=0; i < max_delay; ++i) {
		const int off = (i + roff) % max_delay;
		if (self->memQ[off].size > 0) {
			if (self->memQ[off].reltime < n_samples) {

				if (self->memQ[off].size == 3 && (self->memQ[off].buf[0] & 0xf0) == MIDI_NOTEON) {
					const uint8_t chn = self->memQ[off].buf[0] & 0x0f;
					const uint8_t key = self->memQ[off].buf[1] & 0x7f;
					self->memCS[chn][key]++;
					if (self->memCS[chn][key] > 1) { // send a note-off first
						uint8_t buf[3];
						buf[0] = MIDI_NOTEOFF | chn;
						buf[1] = key; buf[2] = 0;
						forge_midimessage(self, self->memQ[off].reltime, buf, 3);
					}
					forge_midimessage(self, self->memQ[off].reltime, self->memQ[off].buf, self->memQ[off].size);
				}
				else if (self->memQ[off].size == 3 && (self->memQ[off].buf[0] & 0xf0) == MIDI_NOTEOFF) {
					const uint8_t chn = self->memQ[off].buf[0] & 0x0f;
					const uint8_t key = self->memQ[off].buf[1] & 0x7f;
					if (self->memCS[chn][key] > 0) {
						self->memCS[chn][key]--;
						if (self->memCS[chn][key] == 0) {
							forge_midimessage(self, self->memQ[off].reltime, self->memQ[off].buf, self->memQ[off].size);
						}
					}
				} else {
					forge_midimessage(self, self->memQ[off].reltime, self->memQ[off].buf, self->memQ[off].size);
				}

				self->memQ[off].size = 0;
				if (!skipped) self->memI[1] = (self->memI[1] + 1) % max_delay;
			} else {
				self->memQ[off].reltime -= n_samples;
				skipped = 1;
			}
		} else if (!skipped) self->memI[1] = off;

		if (off == woff) break;
	}

	self->memI[3] = self->memI[3] + n_samples; // TODO overflow check
}

void filter_init_midistrum(MidiFilter* self) {
	int c,k;
	srandom ((unsigned int) time (NULL));

	self->memI[0] = MAX(self->samplerate / 16.0, 16);
	self->memI[1] = 0; // read-pointer
	self->memI[2] = 0; // write-pointer
	self->memQ = calloc(self->memI[0], sizeof(MidiEventQueue));
	self->memS = calloc(MAX_STRUM_CHORDNOTES, sizeof(MidiEventQueue));

	self->memI[3] = 0; // monotonic time
	self->memI[4] = 0; // collection stattime
	self->memI[5] = 0; // collected notes
	self->memI[6] = 0; // stroke direction

	self->preproc_fn = filter_preproc_midistrum;
	self->postproc_fn = filter_postproc_midistrum;
	self->cleanup_fn = filter_cleanup_midistrum;

	for (c=0; c < 16; ++c) for (k=0; k < 127; ++k) {
		self->memCS[c][k] = 0; // count note-on per key
	}
}

#endif
