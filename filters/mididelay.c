MFD_FILTER(mididelay)

#ifdef MX_TTF

	mflt:mididelay
	TTF_DEFAULTDEF("MIDI Delayline")
	, TTF_IPORTFLOAT( 0, "delayBPM",  "BPM", 1.0, 280.0,  120.0)
	, TTF_IPORT( 1, "delayBeats", "Delay Beats 4/4", 0.0, 16.0,  1.0, \
			lv2:scalePoint [ rdfs:label "No Delay" ; rdf:value 0.25 ] \
			lv2:scalePoint [ rdfs:label "Eigth Note" ; rdf:value 0.5 ] \
			lv2:scalePoint [ rdfs:label "Quarter Note" ; rdf:value 1.0 ] \
			lv2:scalePoint [ rdfs:label "Half Note" ; rdf:value 2.0 ] \
			lv2:scalePoint [ rdfs:label "Full Note" ; rdf:value 4.0 ] \
			lv2:scalePoint [ rdfs:label "Two Bars " ; rdf:value 8.0 ] \
			lv2:scalePoint [ rdfs:label "Four Bars " ; rdf:value 16.0 ] \
			)
	, TTF_IPORTFLOAT( 2, "delayRandom", "Randomize [Beats]", 0.0, 1.0,  0.0)
	.

#elif defined MX_CODE

static void
filter_cleanup_mididelay(MidiFilter* self)
{
	free(self->memQ);
}

void
filter_midi_mididelay(MidiFilter* self,
		const uint32_t tme,
		const uint8_t* const buffer,
		const uint32_t size)
{
	uint32_t delay = floor(self->samplerate * (*self->cfg[1]) * 60.0 / (*self->cfg[0]));
	float rnd_val = self->samplerate * (*self->cfg[2]) * 60.0 / (*self->cfg[0]);
	float rnd_off = 0;

	if (delay < 0) delay = 0;

	// TODO keep track of note on/off -- use same random delay

	if (rnd_val > 0 && delay > 0) {
		rnd_off -=  MIN(rnd_val, delay);
		rnd_val +=  MIN(rnd_val, delay);
	}
	if (rnd_val > 0) {
		delay += rnd_off + rnd_val * random() / (float)RAND_MAX;
	}

	
	if ((self->memI[2] + 1) % self->memI[0] == self->memI[1]) {
		return;
	}

	MidiEventQueue *qm = &(self->memQ[self->memI[2]]);
	memcpy(qm->buf, buffer, size);
	qm->size = size;
	qm->reltime = tme + delay;
	self->memI[2] = (self->memI[2] + 1) % self->memI[0];
}

static void
filter_postproc_mididelay(MidiFilter* self)
{
	int i;
	const int max_delay = self->memI[0];
	const int roff = self->memI[1];
	const int woff = self->memI[2];
	const uint32_t n_samples = self->n_samples;
	int skipped = 0;

	for (i=0; i < max_delay; ++i) {
		const int off = (i + roff) % max_delay;
		if (self->memQ[off].size > 0) {
			if (self->memQ[off].reltime < n_samples) {
				forge_midimessage(self, self->memQ[off].reltime, self->memQ[off].buf, self->memQ[off].size);
				self->memQ[off].size = 0;
				if (!skipped) self->memI[1] = (self->memI[1] + 1) % max_delay;
			} else {
				self->memQ[off].reltime -= n_samples;
				skipped = 1;
			}
		} else if (!skipped) self->memI[1] = off;

		if (off == woff) break;
	}
}

void filter_init_mididelay(MidiFilter* self) {
	srandom ((unsigned int) time (NULL));
	self->memI[0] = self->samplerate / 16.0;
	self->memI[1] = 0; // read-pointer
	self->memI[2] = 0; // write-pointer
	self->memQ = calloc(self->memI[0], sizeof(MidiEventQueue));
	self->postproc_fn = filter_postproc_mididelay;
	self->cleanup_fn = filter_cleanup_mididelay;
}

#endif
