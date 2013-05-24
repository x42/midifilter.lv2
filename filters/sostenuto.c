MFD_FILTER(sostenuto)

#ifdef MX_TTF

	mflt:sostenuto
	TTF_DEFAULTDEF("MIDI Sostenuto")
	, TTF_IPORTFLOAT( 0, "sostenuto",  "Sostenuto [sec]", 0.0, 60.0,  0.0)
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
		if (off == woff) break;

		if (self->memQ[off].size != 3) continue;
		const uint8_t c = self->memQ[off].buf[0] & 0x0f;
		const uint8_t k = self->memQ[off].buf[1] & 0x7f;
		if (c != chn || k != key) continue;

		if (newdelay >= 0)
			self->memQ[off].reltime = newdelay;
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

void
filter_midi_sostenuto(MidiFilter* self,
		const uint32_t tme,
		const uint8_t* const buffer,
		const uint32_t size)
{
	uint32_t delay = floor(self->samplerate * (*self->cfg[0]));
	if (delay < 0) delay = 0;
	uint8_t mst = buffer[0] & 0xf0;
	const uint8_t vel = buffer[2] & 0x7f;

	if (mst == MIDI_NOTEON && vel ==0 ) {
		mst = MIDI_NOTEOFF;
	}

	if (size == 3 && mst == MIDI_NOTEON) {
		const uint8_t chn = buffer[0] & 0x0f;
		const uint8_t key = buffer[1] & 0x7f;
		if (sostenuto_check_dup(self, chn, key, tme + delay)) {
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
	else if (size == 3 && mst == MIDI_NOTEOFF) {
		const uint8_t chn = buffer[0] & 0x0f;
		const uint8_t key = buffer[1] & 0x7f;

		if (!sostenuto_check_dup(self, chn, key, -1)) {
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

	filter_postproc_sostenuto(self);
}

void filter_init_sostenuto(MidiFilter* self) {
	srandom ((unsigned int) time (NULL));
	self->memI[0] = self->samplerate / 16.0;
	self->memI[1] = 0; // read-pointer
	self->memI[2] = 0; // write-pointer
	self->memQ = calloc(self->memI[0], sizeof(MidiEventQueue));
	self->postproc_fn = filter_postproc_sostenuto;
	self->cleanup_fn = filter_cleanup_sostenuto;
}

#endif
