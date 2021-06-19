#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <cmrx/interface.h>
/** @defgroup bsw Basic Software
 *
 * CMRX offers small palette of services and toolkits serving the purpose of basic 
 * foundation for building higher software layers.
 */

/** @defgroup bsw_com Communication abstraction
 *
 * @ingroup bsw
 *
 * Communication abstraction offers transport-independent way of communication. Abstract 
 * mechanisms offered here cover one-way communication either as a producer
 * or as a consumer and two-way communication. These mechanisms internally use CMRX
 * RPC interfaces, thus actual implementation of communication interfaces is fully
 * up to provider / consumer.
 *
 * Main advantage of abstract communication interfaces is, that it is possible to 
 * change communication channel without need of changing higher layers of the software.
 * This improves portability and reusability of the code.
 *
 * You can use communication abstraction in a following way:
 *
 * Depending on if your communication is uni- or bi-directional and what direction the
 * data flows, choose from one of three interfaces offered here: ComSink, ComSource or
 * ComChannel. Each of these provides pre-defined virtual method table and very lean
 * "virtual" class, which only publishes virtual method table itself. Therefore you are
 * free to provide your own data members and means of how any of these interfaces is 
 * implemented.
 *
 * Next, create your own interface using virtual method table of previously chosen
 * interface, such as:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * struct MySourceImplementation {
 *     const struct ComSourceVMT * vtable;
 *     unsigned int my_member;
 *     unsigned short my_member_two;
 * };
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Next, if you create instance of this interface, you need to initialize the vtable
 * of your interface. You can do it such as:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * // declare, which class will actually be accessible from within methods
 * #define CLASS struct MySourceImplementation
 *
 * // prototypes of methods of interface implementations
 * static int my_source_read(SELF, uint8_t * data, unsigned max_len);
 * static bool my_source_ready(SELF);
 * static void my_source_set_notify(SELF, struct ComNotification * listener, uint32_t signal);
 *
 * // map methods into slots of interface vtable
 * static VTABLE struct ComSourceVMT my_source_vtable = {
 *     &my_source_read,
 *     &my_source_ready,
 *     &my_source_set_notify
 * };
 *
 * // create interface instance
 * static struct MySourceImplementation instance = {
 *     &my_source_vtable,
 *     1,
 *     2
 * };
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * As you can see, You don't need to provide your own declaration of virtual method table.
 * All you need to do is to provide your own methods to be filled into vtable of chosen 
 * interface. Another advantage of this approach is that you can pass instance of 
 * `MySourceImplementation` class whenever, when ComSource is expected. This will work 
 * transparently. Methods you've provided will have access to full details of class 
 * `MySourceImplementation`, while caller will simply treat your class as if it was generic
 * `ComSource` class.
 *
 * @{
 */

/** Helper macro for definition of notification callbacks.
 * This macro defines all the stuff required for notification callback implementation.
 * It defines virtual method table owned by the module and prototypes one method of 
 * this virtual method table. User can start defining body of this method directly after
 * using macro.
 *
 * Typical usage of this macro is:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * static uint32_t notified_channel;
 *
 * COM_NOTIFICATION(my_notification, channel)
 * {
 *     notified_channel = channel;
 *     ...wake up or start thread which handles notification...
 * }
 *
 * ....
 *
 *
 * // channel initialization code:
 *     rpc_call(&channel, set_notify, &my_notification, channel_id);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @param name name of interface instance usable in call to set_notify RPC call
 * @param argname name of argument usable in body of notification handler
 */
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

/// Notification - notify listener of data availability

struct ComNotification;

/** Interface of notification callback.
 */
struct ComNotificationVMT {
	/** Method for notifying external listener of read availability */
	int (*readable_notify)(SELF, uint32_t id);
};

struct ComNotification {
	const struct ComNotificationVMT * vtable;
	Thread_t thread_id;
};

struct ComSource;

/** Methods implemented by the interface of class ComSource
 *
 * These are methods publicly visible in interface ComSource. Each implementation
 * of ComSource interface **must** provide definitions for all of these methods.
 */
struct ComSourceVMT {
	int (*read)(SELF, uint8_t * data, unsigned max_len);
	bool (*ready)(SELF);
	void (*set_notify)(SELF, struct ComNotification * listener, uint32_t signal);
};

/** Source - read end of simplex channel
 *
 * Source serves the purpose of unidirectional source of data. It can be read, but
 * can't be written. User of source can either try to poll the source, or let source
 * implementation know instance of @ref COM_NOTIFICATION macro and be notified, once
 * there is something available to be read from source.
 */
struct ComSource {
	const struct ComSourceVMT * vtable;

};

// Sink - write end of simplex channel

struct ComSink;

/** Methods implemented by the interface of class ComSink
 *
 * These are methods publicly available in interface ComSink. Each implementation
 * of ComSink interface **must** provide definitions for all of these methods.
 */
struct ComSinkVMT {
	int (*write)(SELF, const uint8_t * data, unsigned length);
	bool (*free)(SELF);
};

/** Sink - write end of simplex channel
 *
 * Sink serves the purpose of unidirectional sink for data. It can be written, but
 * cannot be read. Essentially, sink provides only one method, which allows to write
 * a chunk of data into sink. There is no notification available for letting user 
 * to know, if sink is writable, or not.
 */
struct ComSink {
	const struct ComSinkVMT * vtable;
};

// Channel - read / write end of duplex channel

struct ComChannel;

/** Methods implemented by the interface of class ComChannel
 *
 * Interface of class ComChannel is an union of interfaces of ComSink and ComSource.
 * Implementer **must** provide definitions for all the methods present in this
 * interface.
 */
struct ComChannelVMT {
	int (*read)(SELF, uint8_t * data, unsigned max_len);
	bool (*ready)(SELF);
	void (*set_notify)(SELF, struct ComNotification * listener, uint32_t signal);
	int (*write)(SELF, const uint8_t * data, unsigned length);
	bool (*free)(SELF);
};

/** Channel - bidirectional communication channel
 *
 * ComChannel combines features (and interfaces) of ComSink and ComSource. It allows both
 * reading and writing into something, which should be single bidirectional channel.
 * Implementation is free to decide, how the communication will actually be implemented.
 * It may decide to bond one sink and source into one bidirectional channel or implemented
 * bidirectional channel from scratch. Interface of ComChannel is an union of interfaces
 * of ComSource and ComSink.
 */
struct ComChannel {
	const struct ComChannelVMT * vtable;
};

int foo(int bar);

/** @} */
