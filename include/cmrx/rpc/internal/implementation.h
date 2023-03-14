/* Scary internals of how this mechanism allows for multiple 
 * interfaces being defined in one translation unit.
 * Internally, interface counter is maintained, which is incremented
 * each time, when "rpc/implementation.h" is included.
 * Each new inclusion thus allows to define one additional interface.
 * Currently the limit is to define 10 interfaces in single
 * translation unit. It can be increseas by prolonging the code 
 * below.
 */
#if CMRX__INTERFACE__COUNTER == 9
#error "Too many interface implementations in one file! Split implementations of interfaces into multiple files!"
#elif CMRX__INTERFACE__COUNTER == 8
#undef CMRX__INTERFACE__COUNTER
#define CMRX__INTERFACE__COUNTER 9
#elif CMRX__INTERFACE__COUNTER == 7
#undef CMRX__INTERFACE__COUNTER
#define CMRX__INTERFACE__COUNTER 8
#elif CMRX__INTERFACE__COUNTER == 6
#undef CMRX__INTERFACE__COUNTER
#define CMRX__INTERFACE__COUNTER 7
#elif CMRX__INTERFACE__COUNTER == 5
#undef CMRX__INTERFACE__COUNTER
#define CMRX__INTERFACE__COUNTER 6
#elif CMRX__INTERFACE__COUNTER == 4
#undef CMRX__INTERFACE__COUNTER
#define CMRX__INTERFACE__COUNTER 5
#elif CMRX__INTERFACE__COUNTER == 3
#undef CMRX__INTERFACE__COUNTER
#define CMRX__INTERFACE__COUNTER 4
#elif CMRX__INTERFACE__COUNTER == 2
#undef CMRX__INTERFACE__COUNTER
#define CMRX__INTERFACE__COUNTER 3
#elif CMRX__INTERFACE__COUNTER == 1
#undef CMRX__INTERFACE__COUNTER
#define CMRX__INTERFACE__COUNTER 2
#elif CMRX__INTERFACE__COUNTER == 0
#undef CMRX__INTERFACE__COUNTER
#define CMRX__INTERFACE__COUNTER 1
#endif

#ifndef CMRX__INTERFACE__COUNTER
#   define CMRX__INTERFACE__COUNTER 0
#endif

#define CMRX_IMPLEMENTATION_IMPL(type, offs) extern type * __cmrx_interface_type_carrier_ ## offs
#define CMRX_IMPLEMENTATION_HELPER(type, offs) CMRX_IMPLEMENTATION_IMPL(type, offs)

#define CMRX_THIS_IMPL(offs) ({ _Static_assert(__builtin_types_compatible_p(typeof(__cmrx_interface_type_carrier_ ## offs), void *) == 0, "Class declaration missing before defining interface for it"); ((typeof(__cmrx_interface_type_carrier_ ## offs)) this_); })
#define CMRX_THIS_HELPER(offs) CMRX_THIS_IMPL(offs)

#define CMRX_IMPLEMENTATION_TYPE_IMPL(type, iface) "Service " #type " does not implement interface " #iface
#define CMRX_IMPLEMENTATION_TYPE_HELPER(type, iface) CMRX_IMPLEMENTATION_TYPE_IMPL(type, iface)
#define CMRX_CHECK_INTERFACE_MATCH(type, interface) __builtin_types_compatible_p(const interface *, typeof(((type *) 0)->vtable))
