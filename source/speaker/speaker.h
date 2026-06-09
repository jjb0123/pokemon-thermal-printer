#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void speaker_init(void);
void speaker_play_littleroot_town(void);
void speaker_play_littleroot_loop(void);
void speaker_test(void);

#ifdef __cplusplus
}
#endif

#endif // SPEAKER_H