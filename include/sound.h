/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SOUND_H_
#define _SOUND_H_

void sndld(const uint8_t *snd);
void sndout(uint8_t cmd);
void sound_init(void);
void sound_irq_init(void);
void keyboard_init(void);
void sndseq(void);

extern const uint8_t cnsnd[];
extern const uint8_t rpsnd[];
extern const uint8_t pdsnd[];
extern const uint8_t st1snd[];
extern const uint8_t st2snd[];
extern const uint8_t tbsnd[];
extern const uint8_t sbsnd[];
extern const uint8_t acsnd[];
extern const uint8_t alsnd[];
extern const uint8_t ahsnd[];
extern const uint8_t ascsnd[];
extern const uint8_t apsnd[];
extern const uint8_t prhsnd[];
extern const uint8_t schsnd[];
extern const uint8_t ufhsnd[];
extern const uint8_t tihsnd[];
extern const uint8_t lhsnd[];
extern const uint8_t lpksnd[];
extern const uint8_t lsksnd[];
extern const uint8_t swhsnd[];
extern const uint8_t lassnd[];
extern const uint8_t lgsnd[];
extern const uint8_t lshsnd[];
extern const uint8_t sshsnd[];
extern const uint8_t ushsnd[];
extern const uint8_t swssnd[];

#endif

