#pragma once

#include <stdbool.h>

// Source - read end of simplex channel

struct ComSource;

struct ComSourceVMT {
	int (*read)(struct ComSource * self, uint8_t * data, unsigned max_len);
	bool (*ready)(struct ComSource * self);
	void (*set_notify)(struct ComSource * self, uint16_t thread_id, uint32_t signal);
};

struct ComSource {
	const struct ComSourceVMT * vtable;

};

// Sink - write end of simplex channel

struct ComSink;

struct ComSinkVMT {
	int (*write)(struct ComSink * self, const uint8_t * data, unsigned length);
	bool (*free)(struct ComSource * self);
};

struct ComSink {
	const struct ComSinkVMT * vtable;
};

// Channel - read / write end of duplex channel

struct ComChannel;

struct ComChannelVMT {
	int (*read)(struct ComSource * self, uint8_t * data, unsigned max_len);
	bool (*ready)(struct ComSource * self);
	void (*set_notify)(struct ComSource * self, uint16_t thread_id, uint32_t signal);
	int (*write)(struct ComSink * self, const uint8_t * data, unsigned length);
	bool (*free)(struct ComSource * self);
};

struct ComChannel {
	const struct ComChannelVMT * vtable;
};
