/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "wav.h"

int read_wavfile(const char *path, WAVHeader **wavp)
{
    WAVHeader wav;
    uint8_t *buf;
    uint8_t *buf2;
    uint16_t *rp;
    uint8_t *wp;
    uint32_t i;
    ssize_t n;
    int more;
    FILE *fp;

    fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        return -1;
    }
    if (fread(&wav, sizeof(wav), 1, fp) != 1) {
        perror(path);
        return -1;
    }
    if (wav.MasterID != ID_RIFF ||
        wav.FormatType != ID_WAVE ||
        wav.FmtID != ID_FMT ||
        wav.wFormatTag != WAVE_FORMAT_PCM ||
        wav.wChannels != 1 ||
        wav.dwSamplesPerSec != 44100 ||
        (wav.dwAvgBytesPerSec != 44100 && wav.dwAvgBytesPerSec != 88200) ||
        (wav.wBitsPerSample != 8 && wav.wBitsPerSample != 16) ||
        wav.DataID != ID_DATA) {
        fprintf(stderr, "%s: unrecognized format\n", path);
        return -1;
    }
    buf = malloc(sizeof(wav) + wav.DataSize);
    memcpy(buf, &wav, sizeof(wav));
    n = fread(buf + sizeof(wav), 1, wav.DataSize, fp);
    if (n != wav.DataSize) {
        fprintf(stderr, "%s: read(W:%d/G:%ld)\n", path, wav.DataSize, n);
        goto quit;
    }
    more = 0;
    while (fgetc(fp) != EOF) {
        ++more;
    }
    if (more) {
        fprintf(stderr, "%s: WARNING: %d more bytes\n", path, more);
    }
    fclose(fp);

    // convert from 16-bit to 8-bit
    if (wav.wBitsPerSample == 16) {
        buf2 = malloc(sizeof(wav) + wav.DataSize / 2);
        memcpy(buf2, &wav, sizeof(wav));
        ((WAVHeader *)buf2)->dwAvgBytesPerSec = 44100;
        ((WAVHeader *)buf2)->wBlockAlign = 1;
        ((WAVHeader *)buf2)->wBitsPerSample = 8;
        ((WAVHeader *)buf2)->DataSize /= 2;
        rp = (uint16_t *)((WAVHeader *)buf)->buf;
        wp = ((WAVHeader *)buf2)->buf;
        for (i = 0; i < ((WAVHeader *)buf2)->DataSize; i++) {
            *wp++ = ((*rp++) + 0x8000) >> 8;
        }
        free(buf);
        buf = buf2;
    }
    *wavp = (WAVHeader *)buf;
    return 0;
quit:
    free(buf);
    fclose(fp);
    return -1;
}

