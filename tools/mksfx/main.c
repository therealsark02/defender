/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "sfxdata.h"
#include "wav.h"

static const char *progname;
static int verbose = 0;

static const uint16_t oplevel[16][16][16] =
#include "ym2149_fixed_vol.h"

// pcm 0..255 -> 1-,2-,and 3-channel ymvol 0..15
static uint8_t pcm_to_ymvol1[256];
static uint8_t pcm_to_ymvol2[256][2];
static uint8_t pcm_to_ymvol3[256][3];

static void mkpcmtab1(void)
{
    double want, got, nerr, err, ovol, max_ovol;
    int i, avol, best_avol;

    // find the 1-channel max vol
    max_ovol = 0;
    for (avol = 0; avol < 16; avol++) {
        ovol = oplevel[0][0][avol];
        if (ovol > max_ovol) {
            max_ovol = ovol;
        }
    }

    if (verbose) {
        printf("  i     want |  got    err%%   avol\n");
    }
    for (i = 0; i < 256; i++) {
        want = (double)i / 255.0;
        best_avol = 0;
        got = oplevel[0][0][0];
        err = fabs(want - got);
        for (avol = 0; avol < 16; avol++) {
            ovol = oplevel[0][0][avol] / max_ovol;
            nerr = fabs(want - ovol);
            if (nerr < err) {
                best_avol = avol;
                got = ovol;
                err = nerr;
            }
        }
        pcm_to_ymvol1[i] = best_avol;
        err = fabs(100 * (1.0 - want / got));
        if (verbose) {
            printf("[%3d]: %5.3f | %5.3f (%4.1f)    %2d\n",
                i, want, got, err, best_avol);
        }
    }
}

static void mkpcmtab2(void)
{
    double want, got, nerr, err, ovol, max_ovol;
    int i, avol, best_avol, bvol, best_bvol;

    // find the 2-channel max vol
    max_ovol = 0;
    for (avol = 0; avol < 16; avol++) {
        for (bvol = 0; bvol < 16; bvol++) {
            ovol = oplevel[0][bvol][avol];
            if (ovol > max_ovol) {
                max_ovol = ovol;
            }
        }
    }

    if (verbose) {
        printf("  i     want |  got    err%%   avol bvol\n");
    }
    for (i = 0; i < 256; i++) {
        want = (double)i / 255.0;
        best_avol = best_bvol = 0;
        got = oplevel[0][0][0];
        err = fabs(want - got);
        for (avol = 0; avol < 16; avol++) {
            for (bvol = 0; bvol < 16; bvol++) {
                ovol = oplevel[0][bvol][avol] / max_ovol;
                nerr = fabs(want - ovol);
                if (nerr < err) {
                    best_avol = avol;
                    best_bvol = bvol;
                    got = ovol;
                    err = nerr;
                }
            }
        }
        pcm_to_ymvol2[i][0] = best_avol;
        pcm_to_ymvol2[i][1] = best_bvol;
        err = fabs(100 * (1.0 - want / got));
        if (verbose) {
            printf("[%3d]: %5.3f | %5.3f (%4.1f)    %2d   %2d\n",
                i, want, got, err, best_avol, best_bvol);
        }
    }
}

static void mkpcmtab3(void)
{
    double want, got, nerr, err, ovol, max_ovol;
    int i, avol, best_avol, bvol, best_bvol, cvol, best_cvol;

    // find the 3-channel max vol
    max_ovol = 0;
    for (avol = 0; avol < 16; avol++) {
        for (bvol = 0; bvol < 16; bvol++) {
            for (cvol = 0; cvol < 16; cvol++) {
                ovol = oplevel[cvol][bvol][avol];
                if (ovol > max_ovol) {
                    max_ovol = ovol;
                }
            }
        }
    }

    if (verbose) {
        printf("  i     want |  got    err%%   avol bvol cvol\n");
    }
    for (i = 0; i < 256; i++) {
        want = (double)i / 255.0;
        best_avol = best_bvol = best_cvol = 0;
        got = oplevel[0][0][0];
        err = fabs(want - got);
        for (avol = 0; avol < 16; avol++) {
            for (bvol = 0; bvol < 16; bvol++) {
                for (cvol = 0; cvol < 16; cvol++) {
                    ovol = oplevel[cvol][bvol][avol] / max_ovol;
                    nerr = fabs(want - ovol);
                    if (nerr < err) {
                        best_avol = avol;
                        best_bvol = bvol;
                        best_cvol = cvol;
                        got = ovol;
                        err = nerr;
                    }
                }
            }
        }
        pcm_to_ymvol3[i][0] = best_avol;
        pcm_to_ymvol3[i][1] = best_bvol;
        pcm_to_ymvol3[i][2] = best_cvol;
        err = fabs(100 * (1.0 - want / got));
        if (verbose) {
            printf("[%3d]: %5.3f | %5.3f (%4.1f)    %2d   %2d   %2d\n",
                i, want, got, err, best_avol, best_bvol, best_cvol);
        }
    }
}

static void mkpcmtab(void)
{
    mkpcmtab1();
    mkpcmtab2();
    mkpcmtab3();
}

static void outfile_push(FILE *fp, int packed, int *outphase,
        uint8_t *outbyte, uint8_t val)
{
    if (packed) {
        if (*outphase == 0) {
            *outbyte = val << 4;
            *outphase = 1;
        } else {
            fputc(*outbyte | val, fp);
            *outphase = 0;
        }
    } else {
        fputc(val, fp);
    }
}

static int write_sound_file(const WAVHeader *wav, int freq, int flags, int nchans, int len_ms, const char *path)
{
    uint8_t samp, avol, bvol, cvol, outbyte;
    uint32_t nsamples;
    sfx_sound_header_t hdr;
    int packed, outphase;
    double pos, step;
    FILE *fp;

    fp = fopen(path, "w");
    if (fp == NULL) {
        perror(path);
        return -1;
    }

    packed = !!(flags & SFXF_PACKED);
    step = 44100.0 / freq;
    nsamples = wav->DataSize / step;
    if (len_ms >= 0) {
        nsamples = freq * len_ms / 1000;
        if (nsamples > wav->DataSize) {
            nsamples = wav->DataSize;
        }
    }
    hdr.magic = htons(SFX_MAGIC);
    hdr.freq = htons(freq);
    hdr.flags = flags;
    hdr.nchans = nchans;
    hdr.nsamples = htonl(nsamples);
    hdr.len = 0;
    if (fwrite(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        goto err1;
    }
    outphase = 0;
    outbyte = 0;
    for (pos = 0; (int)pos < wav->DataSize; pos += step) {
        if (nsamples-- == 0) {
            break;
        }
        samp = wav->buf[(int)pos];
        switch (nchans) {
        case 1:
            avol = pcm_to_ymvol1[samp];
            outfile_push(fp, packed, &outphase, &outbyte, avol);
            break;
        case 2:
            avol = pcm_to_ymvol2[samp][0];
            bvol = pcm_to_ymvol2[samp][1];
            outfile_push(fp, packed, &outphase, &outbyte, avol);
            outfile_push(fp, packed, &outphase, &outbyte, bvol);
            break;
        case 3:
            avol = pcm_to_ymvol3[samp][0];
            bvol = pcm_to_ymvol3[samp][1];
            cvol = pcm_to_ymvol3[samp][2];
            outfile_push(fp, packed, &outphase, &outbyte, avol);
            outfile_push(fp, packed, &outphase, &outbyte, bvol);
            outfile_push(fp, packed, &outphase, &outbyte, cvol);
            break;
        default:
            abort();
        }
    }
    if (packed && outphase == 1) {
        fputc(outbyte, fp);
    }
    hdr.len = htonl(ftell(fp) - sizeof(hdr));
    rewind(fp);
    if (fwrite(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        goto err1;
    }
    if (fclose(fp) == EOF) {
        goto err2;
    }
    return 0;
err1:
    fclose(fp);
err2:
    perror(path);
    return -1;
}

static int write_ste_sound_file(const WAVHeader *wav, int freq, int flags, int len_ms, const char *path)
{
    sfx_sound_header_t hdr;
    uint32_t nsamples;
    double pos, step;
    uint8_t samp;
    FILE *fp;

    fp = fopen(path, "w");
    if (fp == NULL) {
        perror(path);
        return -1;
    }

    step = 44100.0 / freq;
    nsamples = wav->DataSize / step;
    if (len_ms >= 0) {
        nsamples = freq * len_ms / 1000;
        if (nsamples > wav->DataSize) {
            nsamples = wav->DataSize;
        }
    }
    hdr.magic = htons(SFX_MAGIC);
    hdr.freq = htons(freq);
    hdr.flags = flags;
    hdr.nchans = 1;
    hdr.nsamples = htonl(nsamples);
    hdr.len = 0;
    if (fwrite(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        goto err1;
    }
    for (pos = 0; (int)pos < wav->DataSize; pos += step) {
        if (nsamples-- == 0) {
            break;
        }
        samp = wav->buf[(int)pos];
        // 0..255 -> -128..+127
        fputc((samp - 0x80) & 0xff, fp);
    }
    hdr.len = htonl(ftell(fp) - sizeof(hdr));
    rewind(fp);
    if (fwrite(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        goto err1;
    }
    if (fclose(fp) == EOF) {
        goto err2;
    }
    return 0;
err1:
    fclose(fp);
err2:
    perror(path);
    return -1;
}
static int usage(void)
{
    fprintf(stderr, "usage: %s [-f freq] [-p(acked)] [-l len_ms] [-c nchans] [-o outfile] infile\n",
            progname);
    return 1;
}

int main(int argc, char *argv[])
{
    const char *outpath = "effect.bin";
    int freq = 9600;
    int len = -1; /* full */
    int ste_mode = 0;
    int flags = 0;
    int nchans = 1;
    WAVHeader *wav;
    int c;

    progname = argv[0];
    mkpcmtab();
    while ((c = getopt(argc, argv, "sf:o:l:prc:")) != -1) {
        switch (c) {
        case 's':
            ste_mode = 1;
            break;
        case 'r':
            flags |= SFXF_REPEAT;
            break;
        case 'f':
            freq = atoi(optarg);
            break;
        case 'o':
            outpath = optarg;
            break;
        case 'l':
            len = atoi(optarg);
            break;
        case 'p':
            flags |= SFXF_PACKED;
            break;
        case 'c':
            nchans = atoi(optarg);
            if (nchans < 1 || nchans > 3) {
                fprintf(stderr, "-c must be in range 1..3\n");
                return 1;
            }
            break;
        default:
            return usage();
        }
    }
    argc -= optind;
    argv += optind;
    if (argc < 1) {
        return usage();
    }
    if (read_wavfile(argv[0], &wav) < 0) {
        return 1;
    }
    if (ste_mode) {
        return write_ste_sound_file(wav, freq, flags, len, outpath);
    } else {
        return write_sound_file(wav, freq, flags, nchans, len, outpath);
    }
}

