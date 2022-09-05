#ifndef __BASE_H__
#define __BASE_H__

#include <stdint.h>

enum ERR_CODE_TYPE{
	ERR_OK = 0,
	ERR_FAILED = 1,
	ERR_INIT_FAILED = 2,
};

#define KBDEXT (USHORT)0x0100
/* keyboard Flags */
#define KBD_FLAGS_EXTENDED 0x0100
#define KBD_FLAGS_EXTENDED1 0x0200
#define KBD_FLAGS_DOWN 0x4000
#define KBD_FLAGS_RELEASE 0x8000

/* Pointer Flags */
#define PTR_FLAGS_HWHEEL 0x0400
#define PTR_FLAGS_WHEEL 0x0200
#define PTR_FLAGS_WHEEL_NEGATIVE 0x0100
#define PTR_FLAGS_MOVE 0x0800
#define PTR_FLAGS_DOWN 0x8000
#define PTR_FLAGS_BUTTON1 0x1 /* left */
#define PTR_FLAGS_BUTTON2 0x2 /* middle */
#define PTR_FLAGS_BUTTON3 0x3 /* right */

#define AGORA_SCANCODE_CODE(_agora_scancode) ((BYTE)(_agora_scancode & 0xFF))
#define MAKE_AGORA_SCANCODE(_code, _extended) (((_code)&0xFF) | ((_extended) ? KBDEXT : 0)) 
#define AGORA_SCANCODE_EXTENDED(_agora_scancode) (((_agora_scancode) & KBDEXT) ? TRUE : FALSE)

#endif

