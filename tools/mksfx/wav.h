/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _WAV_H_
#define _WAV_H_

typedef struct {
    // Master
    uint32_t MasterID;
    uint32_t MasterSize;
    uint32_t FormatType;
    // Format
    uint32_t FmtID;
    uint32_t FmtSize;
    uint16_t wFormatTag;
    uint16_t wChannels;
    uint32_t dwSamplesPerSec;
    uint32_t dwAvgBytesPerSec;
    uint16_t wBlockAlign;
    uint16_t wBitsPerSample;
    // Data
    uint32_t DataID;
    uint32_t DataSize;
    uint8_t  buf[0];
} WAVHeader;
#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746D66
#define ID_DATA 0x61746164
#define WAVE_FORMAT_PCM     0x0001

int read_wavfile(const char *path, WAVHeader **wp);

#endif

