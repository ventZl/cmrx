#pragma once

#include <stdbool.h>
#include <cmrx/interface.h>

// Source - read end of simplex channel

struct ComSource;

struct ComSourceVMT {
	int (*read)(SELF, uint8_t * data, unsigned max_len);
	bool (*ready)(SELF);
	void (*set_notify)(SELF, uint16_t thread_id, uint32_t signal);
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
	void (*set_notify)(SELF, uint16_t thread_id, uint32_t signal);
	int (*write)(SELF, const uint8_t * data, unsigned length);
	bool (*free)(SELF);
};

struct ComChannel {
	const struct ComChannelVMT * vtable;
};
