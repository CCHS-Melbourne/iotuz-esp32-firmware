#ifndef _IOTUZ_GRAPHICS_H_
#define _IOTUZ_GRAPHICS_H_

#define TFT_DC     4
#define TFT_CS    19
#define TFT_MOSI  13
#define TFT_CLK   14
#define TFT_RST   32
#define TFT_MISO  12

#ifdef __cplusplus
extern "C" {
#endif

void iotuz_graphics_initialize(void);

#ifdef __cplusplus
}
#endif
#endif
