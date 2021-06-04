#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <cmrx/interface.h>

// Notification - notify listener of data availability

struct ComNotification;

struct ComNotificationVMT {
	int (*readable_notify)(SELF, uint32_t id);
};

struct ComNotification {
	const struct ComNotificationVMT * vtable;
	Thread_t thread_id;
};

#define COM_NOTIFICATION(name, argname) \
static int __ ## name ## _handler(SELF, uint32_t argname);\
\
static VTABLE struct ComNotificationVMT name ## _vmt = {\
	&__ ## name ## _handler\
};\
\
static struct ComNotification name = {\
	& name ## _vmt,\
	0xFF\
};\
\
static int __ ## name ## _handler(SELF, uint32_t argname)

// Source - read end of simplex channel

struct ComSource;

struct ComSourceVMT {
	int (*read)(SELF, uint8_t * data, unsigned max_len);
	bool (*ready)(SELF);
	void (*set_notify)(SELF, struct ComNotification * listener, uint32_t signal);
};

struct ComSource {
	const struct ComSourceVMT * vtable;

};

// Sink - write end of simplex channel

struct ComSink;

struct ComSinkVMT {
	int (*write)(SELF, const uint8_t * data, unsigned length);
	bool (*free)(SELF);
};

struct ComSink {
	const struct ComSinkVMT * vtable;
};

// Channel - read / write end of duplex channel

struct ComChannel;

struct ComChannelVMT {
	int (*read)(SELF, uint8_t * data, unsigned max_len);
	bool (*ready)(SELF);
	void (*set_notify)(SELF, struct ComNotification * listener, uint32_t signal);
	int (*write)(SELF, const uint8_t * data, unsigned length);
	bool (*free)(SELF);
};

struct ComChannel {
	const struct ComChannelVMT * vtable;
};
