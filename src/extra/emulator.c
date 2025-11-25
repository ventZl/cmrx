#include <extra/emulator.h>
#include <cmrx/util.h>

struct DummyPeripheral_t {
    volatile uint32_t reg;
};

struct __attribute__((packed)) EmuDummyPeripheral_t {
    CMRX_Peripheral_Emulator_t emu;
    struct DummyPeripheral_t device;
};

static CMRX_Peripheral_Emulator_t * cmrx_find_emulator(void * instance)
{
    struct DummyPeripheral_t * dummy = (struct DummyPeripheral_t *) instance;
    struct EmuDummyPeripheral_t * emu = CONTAINER_OF(dummy, struct EmuDummyPeripheral_t, device);
    return &emu->emu;
}

uint8_t cmrx_emu_reg_R8(void * instance, const volatile uint8_t * source)
{
    cmrx_find_emulator(instance)->emu_read_cb(source);
    return *source;
}

uint16_t cmrx_emu_reg_R16(void * instance, const volatile uint16_t * source)
{
    cmrx_find_emulator(instance)->emu_read_cb(source);
    return *source;
}

uint32_t cmrx_emu_reg_R32(void * instance, const volatile uint32_t * source)
{
    cmrx_find_emulator(instance)->emu_read_cb(source);
    return *source;
}


void cmrx_emu_reg_W8(void * instance, volatile uint8_t * dest, uint8_t value)
{
    *dest = value;
    cmrx_find_emulator(instance)->emu_write_cb(dest);
}

void cmrx_emu_reg_W16(void * instance, volatile uint16_t * dest, uint16_t value)
{
    *dest = value;
    cmrx_find_emulator(instance)->emu_write_cb(dest);
}

void cmrx_emu_reg_W32(void * instance, volatile uint32_t * dest, uint32_t value)
{
    *dest = value;
    cmrx_find_emulator(instance)->emu_write_cb(dest);
}

