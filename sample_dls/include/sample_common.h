
#ifndef _SAMPLE_COMMON_H_
#define _SAMPLE_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define JPEG_BUF_MAX (128*1024) // 480x272
#define ACC_BUF_MAX 1024

typedef int (*SampleCallBack)(uint8_t *data, int size);

int get_sample_dls_video_len(void);
uint8_t *get_sample_dls_video_buf(void);
void set_sample_dls_video_callback(SampleCallBack set, SampleCallBack get);

int get_sample_dls_audio_len(void);
uint8_t *get_sample_dls_audio_buf(void);
void set_sample_dls_audio_callback(SampleCallBack set, SampleCallBack get);

void sample_nng_set_video_size(int width, int height);
int sample_nng_load(int DLS_VO, int DLS_VI);
int sample_nng_free(int DLS_VO, int DLS_VI);
void sample_nng_signalHandler(int signum);
int get_sample_nng_exit_flag(void);

#ifdef __cplusplus
}
#endif

#endif  /* _SAMPLE_COMMON_H_ */
