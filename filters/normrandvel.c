MFD_FILTER(normrandvel)

#ifdef MX_TTF

    mflt:normrandvel
    TTF_DEFAULTDEF("MIDI Velocity Randomization (Normal)")
    , TTF_IPORT(0, "channel", "Filter Channel",  0.0, 16.0,  0.0, PORTENUMZ("Any"))
    , TTF_IPORTFLOAT(1, "dev", "Velocity Standard Deviation", 0.0, 64.0, 8.0)
    .

#elif defined MX_CODE

static void filter_init_normrandvel(MidiFilter* self) {
    srandom ((unsigned int) time (NULL));
    self->memF[0] = 2*random() / (float)RAND_MAX - 1;
}


//calculates random numbers according to a modified Marsaglia polar method,
// approaching a normal distribution, should be more "human"
// User sets standard dev.
static void
filter_midi_normrandvel(MidiFilter* self,
        uint32_t tme,
        const uint8_t* const buffer,
        uint32_t size)
{
    const int chs = midi_limit_chn(floor(*self->cfg[0]) -1);
    const int chn = buffer[0] & 0x0f;
    int mst = buffer[0] & 0xf0;

    if (!size == 3
            || !(mst == MIDI_NOTEON || (mst == MIDI_NOTEOFF))
            || !(floor(*self->cfg[0]) == 0 || chs == chn)
            )
    {
        forge_midimessage(self, tme, buffer, size);
        return;
    }

    const uint8_t vel = buffer[2] & 0x7f;

    if (mst == MIDI_NOTEON && vel ==0 ) {
        mst = MIDI_NOTEOFF;
    }

    uint8_t buf[3];
    buf[0] = buffer[0];
    buf[1] = buffer[1];

    const float dev = *(self->cfg[1]);

    float U = 2.0* random() / (float)RAND_MAX - 1;//rand E(-1,1)
    float S = SQUARE(U) + SQUARE(self->memF[0]);//map 2 random vars to unit circle
    uint8_t timeout = 0;
    while(S>=1)//repull RV if outside unit circle
    {
        if(timeout++>2)
        {
            U = 2.0* random() / (float)RAND_MAX - 1;
            S = SQUARE(U) + SQUARE(self->memF[0]);
        }else{//guarantee an exit, velocity will be unchanged
            U=0;
        }
    }

    self->memF[0] = U;//store RV for next round

    const float rnd = U ? (dev*U*sqrt(-2*log(S)/S)) : 0;

    switch (mst) {
        case MIDI_NOTEON:
            buf[2] = RAIL(rintf(buffer[2] + rnd), 1, 127);
            break;
        case MIDI_NOTEOFF:
            buf[2] = RAIL(rintf(buffer[2] + rnd), 0, 127);
            break;
    }

    forge_midimessage(self, tme, buf, size);
}

#endif
