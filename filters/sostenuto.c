MFD_FILTER(sostenuto)

#ifdef MX_TTF

	mflt:sostenuto
	TTF_DEFAULTDEF("MIDI Sostenuto")
	, TTF_IPORT( 0, "sostenuto",  "Sostenuto [sec]", 0.0, 600.0,  0.0, units:unit units:s)
	, TTF_IPORTTOGGLE( 1, "pedal",  "Enable", 1.0)
	rdfs:comment "This filter delays note-off messages by a given time, emulating a piano sostenuto pedal."
	.

#elif defined MX_CODE

static void
filter_cleanup_sostenuto(MidiFilter* self)
{
	free(self->memQ);
}

static int sostenuto_check_dup(MidiFilter* self,
		uint8_t chn,
		uint8_t key,
		int newdelay
		) {
	int i;
	const int max_delay = self->memI[0];
	const int roff = self->memI[1];
	const int woff = self->memI[2];

	for (i=0; i < max_delay; ++i) {
		const int off = (i + roff) % max_delay;

		if (self->memQ[off].size != 3) {
			if (off == woff) break;
			continue;
		}

		const uint8_t c = self->memQ[off].buf[0] & 0x0f;
		const uint8_t k = self->memQ[off].buf[1] & 0x7f;
		if (c != chn || k != key) {
			if (off == woff) break;
			continue;
		}

		if (newdelay >= 0)
			self->memQ[off].reltime = newdelay;
		else
			self->memQ[off].size = 0;
		return 1;
	}
	return 0;
}

static void
filter_postproc_sostenuto(MidiFilter* self)
{
	int i;
	const int max_delay = self->memI[0];
	const int roff = self->memI[1];
	const int woff = self->memI[2];
	uint32_t n_samples = self->n_samples;
	int skipped = 0;

	/* only process until given time */
	if (self->memI[3] > 0)
		n_samples = MIN(self->memI[3], n_samples);

	for (i=0; i < max_delay; ++i) {
		const int off = (i + roff) % max_delay;
		if (self->memQ[off].size > 0) {
			if (self->memQ[off].reltime < n_samples) {
				forge_midimessage(self, self->memQ[off].reltime, self->memQ[off].buf, self->memQ[off].size);
				self->memQ[off].size = 0;
				if (!skipped) self->memI[1] = (self->memI[1] + 1) % max_delay;
			} else {
				/* don't decrement time if called from filter_midi_sostenuto() */
				if (self->memI[3] < 0) {self->memQ[off].reltime -= n_samples;}
				skipped = 1;
			}
		} else if (!skipped) self->memI[1] = off;

		if (off == woff) break;
	}

}

void
filter_midi_sostenuto(MidiFilter* self,
		const uint32_t tme,
		const uint8_t* const buffer,
		const uint32_t size)
{
	const uint32_t delay = floor(self->samplerate * RAIL((*self->cfg[0]), 0, 120));
	const int state = RAIL(*self->cfg[1], 0, 1);

	uint8_t mst = buffer[0] & 0xf0;
	const uint8_t vel = buffer[2] & 0x7f;

	if (mst == MIDI_NOTEON && vel ==0 ) {
		mst = MIDI_NOTEOFF;
	}

	if (size == 3 && mst == MIDI_NOTEON && state == 1) {
		const uint8_t chn = buffer[0] & 0x0f;
		const uint8_t key = buffer[1] & 0x7f;
		if (sostenuto_check_dup(self, chn, key, -1)) {
			/* note was already on,
			 * send note off + immediate note on, again
			 */
			uint8_t buf[3];
			buf[0] = MIDI_NOTEOFF | chn;
			buf[1] = key;
			buf[2] = 0;
			forge_midimessage(self, tme, buf, size);
		}
		forge_midimessage(self, tme, buffer, size);
	}
	else if (size == 3 && mst == MIDI_NOTEOFF && state == 1) {
		const uint8_t chn = buffer[0] & 0x0f;
		const uint8_t key = buffer[1] & 0x7f;

		if (!sostenuto_check_dup(self, chn, key, tme + delay)) {
			// queue note-off if not already queued
			MidiEventQueue *qm = &(self->memQ[self->memI[2]]);
			memcpy(qm->buf, buffer, size);
			qm->size = size;
			qm->reltime = tme + delay;
			self->memI[2] = (self->memI[2] + 1) % self->memI[0];
		}
	}
	else {
		forge_midimessage(self, tme, buffer, size);
	}

	/* only note-off are queued in the buffer.
	 * Interleave delay-buffer with messages sent in-process above
	 * to retain sequential order.
	 */
	self->memI[3] = tme + 1;
	filter_postproc_sostenuto(self);
	self->memI[3] = -1;
}

static void
filter_preproc_sostenuto(MidiFilter* self)
{
	int i;
	const int max_delay = self->memI[0];
	const int roff = self->memI[1];
	const int woff = self->memI[2];
	const int state = RAIL(*self->cfg[1], 0, 1);

	if (   self->lcfg[0] == *self->cfg[0]
			&& self->lcfg[1] == *self->cfg[1]) {
		return;
	}

	const float diff = *self->cfg[0] - self->lcfg[0];
	const int delay = rint(self->samplerate * diff);

	for (i=0; i < max_delay; ++i) {
		const int off = (i + roff) % max_delay;
		if (self->memQ[off].size > 0) {
			if (state == 0) {
				self->memQ[off].reltime = 0;
			} else {
				self->memQ[off].reltime = MAX(0, self->memQ[off].reltime + delay);
			}
		}
		if (off == woff) break;
	}

	self->memI[3] = 1;
	filter_postproc_sostenuto(self);
	self->memI[3] = -1;
}

void filter_init_sostenuto(MidiFilter* self) {
	srandom ((unsigned int) time (NULL));
	self->memI[0] = self->samplerate / 16.0;
	self->memI[1] = 0; // read-pointer
	self->memI[2] = 0; // write-pointer
	self->memI[3] = -1; // max time-offset
	self->memQ = calloc(self->memI[0], sizeof(MidiEventQueue));
	self->postproc_fn = filter_postproc_sostenuto;
	self->preproc_fn = filter_preproc_sostenuto;
	self->cleanup_fn = filter_cleanup_sostenuto;
}

#endif
