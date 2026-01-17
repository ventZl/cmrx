#include <kernel/mpu.h>
#include <cmrx/defines.h>
#include <arch/mpu_priv.h>
#include <conf/kernel.h>
#include <arch/cortex.h>
#include <arch/memory.h>
#include <cmrx/assert.h>
#include <arch/scb.h>
#include <kernel/sched.h>
#include <kernel/syscall.h>

#ifdef SEMIHOSTING
#include <stdio.h>
#endif

/** @ingroup arch_arm_mpu 
 * @{
 */

#if !defined(__ARM_ARCH_8M_BASE__) && !defined(__ARM_ARCH_8M_MAIN__)

/** MPU region access rights for ARMv6M/ARMv7M.
 * This array maps CMRX access modes to ARM RASR attributes
 * See @ref MPU_Flags for meaning of individual indices.
 */
static const uint32_t __MPU_flags[] = {
	0,
	MPU_RASR_ATTR_AP_PRW_URO,
	MPU_RASR_ATTR_AP_PRW_URW,
	MPU_RASR_ATTR_XN | MPU_RASR_ATTR_AP_PRW_URO,
	MPU_RASR_ATTR_XN | MPU_RASR_ATTR_AP_PRW_URW,
};

#else

/** MPU region access rights for ARMv8M.
 * Stores RBAR and attribute index for each access mode.
 * See @ref MPU_Flags for meaning of individual indices.
 */
static const struct {
	uint32_t rbar_flags;  /* AP, XN, SH bits for RBAR */
	uint8_t attr_idx;     /* MAIR attribute index */
} __MPU_v8m_flags[] = {
	/* MPU_NONE */  { 0, 0 },
	/* MPU_RX */    { MPU_RBAR_AP_RW_RO, MPU_ATTR_NORMAL_WB },                    /* Priv RW, User RO, executable */
	/* MPU_RWX */   { MPU_RBAR_AP_RW_RW, MPU_ATTR_NORMAL_WB },                    /* Priv RW, User RW, executable */
	/* MPU_R */     { MPU_RBAR_XN | MPU_RBAR_AP_RW_RO, MPU_ATTR_NORMAL_WB },     /* Priv RW, User RO, no-execute */
	/* MPU_RW */    { MPU_RBAR_XN | MPU_RBAR_AP_RW_RW, MPU_ATTR_NORMAL_WB },     /* Priv RW, User RW, no-execute */
};

#endif

/** Handler for hard fault.
 */
void hard_fault_handler(void)
{
	uint32_t status = SCB_CFSR;
	if ((status & SCB_CFSR_IACCVIOL) || (status & SCB_CFSR_DACCVIOL))
	{
		if (status & SCB_CFSR_MMARVALID)
		{
#ifdef SEMIHOSTING
			uint32_t addr = SCB_MMFAR;
			printf("Segmentation fault at address 0x%08X in thread %d\n", (uint32_t) addr, os_get_current_thread());
			// fail here
			ASSERT(addr == ~addr);
#endif

		}
	}
	ASSERT(0);
}

/** Enable memory protection.
 * This routine enables memory protection with standard memory setup
 * for kernel purposes. This means that any privileged code has full
 * access to memory as if no memory protection was turned on.
 * @note It is safe to call this routine in kernel context even if no
 * memory regions are set.
 */
static inline void mpu_enable()
{
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
	/* ARMv8M: Initialize MAIR registers with memory attributes */
	MPU->MAIR0 =
		(0x00 << (MPU_ATTR_DEVICE_nGnRnE * 8))  |  /* Device memory */
		(0xAA << (MPU_ATTR_NORMAL_WT * 8))      |  /* Normal, Write-Through, RA */
		(0xFF << (MPU_ATTR_NORMAL_WB * 8))      |  /* Normal, Write-Back, RWA */
		(0x44 << (MPU_ATTR_NORMAL_NC * 8));        /* Normal, Non-Cacheable */
	MPU->MAIR1 = 0;
#endif
	MPU->CTRL |= MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE;
}

/** Disable memory protection.
 * This routine will disable memory protection even for unprivileged
 * code.
 */
static inline void mpu_disable()
{
	MPU->CTRL &= ~(MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE);
}

/** Store MPU settings.
 * Stores MPU settings for default amount of regions into off-CPU
 * buffer. This is suitable for store-resume during task switching.
 * @param hosted_state MPU state buffer for the current host process
 * @param parent_state MPU state buffer for the parent process
 */
int mpu_store(MPU_State * hosted_state, MPU_State * parent_state)
{
	if (hosted_state == NULL || parent_state == NULL)
		return E_INVALID_ADDRESS;

#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
	/* ARMv8M: save RBAR and RLAR */
	for (int q = 0; q < MPU_HOSTED_STATE_SIZE; ++q)
	{
		MPU->RNR = ((q << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		(*hosted_state)[q]._MPU_RBAR = MPU->RBAR;
		(*hosted_state)[q]._MPU_RLAR = MPU->RLAR;
	}
	for (int q = MPU_HOSTED_STATE_SIZE; q < MPU_STATE_SIZE; ++q)
	{
		MPU->RNR = ((q << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		(*parent_state)[q]._MPU_RBAR = MPU->RBAR;
		(*parent_state)[q]._MPU_RLAR = MPU->RLAR;
	}
#else
	/* ARMv6M/ARMv7M: save RBAR and RASR */
	for (int q = 0; q < MPU_HOSTED_STATE_SIZE; ++q)
	{
		MPU->RNR = ((q << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		(*hosted_state)[q]._MPU_RBAR = MPU->RBAR;
		(*hosted_state)[q]._MPU_RASR = MPU->RASR;
	}
	for (int q = MPU_HOSTED_STATE_SIZE; q < MPU_STATE_SIZE; ++q)
	{
		MPU->RNR = ((q << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		(*parent_state)[q]._MPU_RBAR = MPU->RBAR;
		(*parent_state)[q]._MPU_RASR = MPU->RASR;
	}
#endif

	return E_OK;
}

int mpu_restore(const MPU_State * hosted_state, const MPU_State * parent_state)
{
	int rv;
	if ((rv = mpu_load(hosted_state, 0, MPU_HOSTED_STATE_SIZE)) != E_OK)
	{
		return rv;
	}
	return mpu_load(parent_state, MPU_HOSTED_STATE_SIZE, MPU_STATE_SIZE - MPU_HOSTED_STATE_SIZE);
}

int mpu_load(const MPU_State * state, uint8_t base, uint8_t count)
{
	if (state == NULL)
		return E_INVALID_ADDRESS;

#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
	/* ARMv8M: restore RBAR and RLAR */
	for (int q = 0; q < count; ++q)
	{
		MPU->RNR = (((base + q) << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		MPU->RBAR = (*state)[base + q]._MPU_RBAR;
		MPU->RLAR = (*state)[base + q]._MPU_RLAR;
	}
#else
	/* ARMv6M/ARMv7M: restore RBAR and RASR */
	for (int q = 0; q < count; ++q)
	{
		MPU->RNR = (((base + q) << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		MPU->RBAR = (*state)[base + q]._MPU_RBAR;
		MPU->RASR = (*state)[base + q]._MPU_RASR;
	}
#endif

	return E_OK;
}

#if !defined(__ARM_ARCH_8M_BASE__) && !defined(__ARM_ARCH_8M_MAIN__)

uint32_t __mpu_expand_class(uint8_t class)
{
	if (class < sizeof(__MPU_flags)/sizeof(__MPU_flags[0]))
	{
		return __MPU_flags[class];
	}
	return 0;
}

#endif
int mpu_configure_region(uint8_t region, const void * base, uint32_t size, uint8_t cls, struct MPU_Registers * region_def);


int mpu_set_region(uint8_t region, const void * base, uint32_t size, uint8_t cls)
{
	struct MPU_Registers config;
	int rv;
	if ((rv = mpu_configure_region(region, base, size, cls, &config)) == E_OK)
	{
		__ISB();
		__DSB();
		MPU->RNR = ((region << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		MPU->RBAR = config._MPU_RBAR;
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
		MPU->RLAR = config._MPU_RLAR;
#else
		MPU->RASR = config._MPU_RASR;
#endif
		__ISB();
		__DSB();
	}
	return rv;
}

#if !defined(__ARM_ARCH_8M_BASE__) && !defined(__ARM_ARCH_8M_MAIN__)

/* ARMv6M/ARMv7M MPU configuration using RBAR/RASR (base + size model) */
int mpu_configure_region(uint8_t region, const void * base, uint32_t size, uint8_t cls, struct MPU_Registers * region_def)
{
	uint8_t regszbits = ((sizeof(uint32_t)*8) - 1) - __builtin_clz(size) - 1;
	uint32_t subregions = 0xFF;

	uint32_t flags;
	if (cls < sizeof(__MPU_flags)/sizeof(__MPU_flags[0]))
	{
		flags = __MPU_flags[cls];
	}
	else
	{
		return E_INVALID;
	}
	// In below code we don't need to take special care about
	// bits above regszbits, because we know they are all zeroes.
	if (size == 0)
	{
		region_def->_MPU_RBAR = ((region << MPU_RBAR_REGION_LSB) & MPU_RBAR_REGION)
		| (((uint32_t) base) & MPU_RBAR_ADDR);

//		MPU_RNR = ((region << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		region_def->_MPU_RASR = 0;
		return E_OK;
	}

	if ((size & ((1 << regszbits) - 1)) != 0)
	{
//		ASSERT(0);
		// for now simply ignore this case
//		return E_MISALIGNED;

		/* Here we have the case, that size is not a power of two.
		 * In such case, there is a possibility of using subregions.
		 * Each MPU region can be divided into 8 subregions and each
		 * subregion can be marked as active - part of region, or 
		 * inactive - not part of region. 
		 *
		 * Thus this code will first find out, if region size is at least 
		 * 1/8 fraction of nearest larger power of two which would completely
		 * accomodate this region. If yes, then it will be determined, if
		 * considering the base address, region completely lies within
		 * one power-of-two aligned block. If yes, then subregions are 
		 * configured. Otherwise error is raised. */

		/* Lets increase regszbits by one, to round it to nearest
		 * larger power of two and investigate everything against
		 * this block size.
		 */
		regszbits++;

		if ((size % (1 << (regszbits - 3))) != 0)
		{
			/* Block size is not divisible by 1/8 of calculated block size.
			 * This means that we are not able to correctly divide superblock
			 * into 1/8 sub blocks to accomodate this region. Return with fault.
			 */
			ASSERT(0);
			return E_WRONG_SIZE;
		}

		/* Now we know, that block can be accomodated in subregions.
		 * We have to find out if base address is allright - it must be
		 * divisible by subregion size.
		 */

		if (((uint32_t) base & ((1 << (regszbits - 3)) - 1)) != 0)
		{
			ASSERT(0);
			return E_MISALIGNED;
		}

		/* Now we know, that base address is at least 1/8 of superblock
		 * aligned. As a next step, we have to find out, if the region on
		 * itself will fit into single superblock, which itself is
		 * superblock aligned.
		 */

		uint32_t base_misalignment = ((uint32_t) base & ((1 << regszbits) - 1)) >> (regszbits - 3);
		uint32_t size_subregions = size >> (regszbits - 3);

		/* Here we check if, considering the base address misalignment
		 * WRT superblock, we can fit the whole region into one superblock.
		 */
		if ((base_misalignment + size_subregions) > 7)
		{
			ASSERT(0);
			return E_MISALIGNED;
		}

		/* At this point we know that:
		 * 0. there is a number (call it B), which is a power of two
		 * 1. block size is not power of two (B nor any other)
		 * 2. block size is a number divisible by B/8
		 * 3. block base is aligned to at least B/8
		 * 4. block completely resides within one memory area which is B-aligned
		 *
		 * These conditions are sufficient for creating a MPU allocation for it.
		 * All we have to do now is to compute subregion occupation.
		 */

		/* This magic subformula will set all bits between base_migalignment-th and
		 * (base_misalignment + size_subregion)-th bit inclusive.
		 */
#if 0		
		subregions = 
			(1 << ((size_subregions + base_misalignment + 1) - 1)) 
			& ~((1 << base_misalignment) - 1);
#endif
	}
	else
	{
		/* Just check, that address is size-aligned */
		if (((uint32_t) base & (( 1 << regszbits) - 1)) != 0)
		{
			ASSERT(0);
			return E_MISALIGNED;
		}
	}

	flags &= (MPU_RASR_ATTR_XN | MPU_RASR_ATTR_AP);

	subregions ^= 0xFF;

	/* Configure region base address */
	region_def->_MPU_RBAR = ((region << MPU_RBAR_REGION_LSB) & MPU_RBAR_REGION)
		| (((uint32_t) base) & MPU_RBAR_ADDR) | MPU_RBAR_VALID;

	region_def->_MPU_RASR = ((regszbits << MPU_RASR_SIZE_LSB) & MPU_RASR_SIZE)
		| ((subregions << MPU_RASR_SRD_LSB) & MPU_RASR_SRD)
		| (flags & (MPU_RASR_ATTR_AP | MPU_RASR_ATTR_XN))
		| MPU_RASR_ATTR_C
		| MPU_RASR_ENABLE;

	return E_OK;

}

#else

/* ARMv8M MPU configuration using RBAR/RLAR (base + limit model) */
int mpu_configure_region(uint8_t region, const void * base, uint32_t size, uint8_t cls, struct MPU_Registers * region_def)
{
	/* Validate class */
	if (cls >= sizeof(__MPU_v8m_flags)/sizeof(__MPU_v8m_flags[0]))
	{
		return E_INVALID;
	}

	/* Handle disabled region */
	if (size == 0)
	{
		region_def->_MPU_RBAR = 0; // AI put here: (((uint32_t) base) & MPU_RBAR_ADDR_Msk);
		region_def->_MPU_RLAR = 0;  /* Disabled */
		return E_OK;
	}

	/* ARMv8M requires 32-byte minimum alignment */
	if (((uint32_t) base & 0x1F) != 0)
	{
		ASSERT(0);
		return E_MISALIGNED;
	}

	/* Calculate limit address (base + size - 1), must be 32-byte aligned */
	uint32_t limit = ((uint32_t) base) + size - 1;

	/* Verify limit doesn't overflow and maintains alignment */
	if (limit < (uint32_t) base || (limit & 0x1F) != 0x1F)
	{
		ASSERT(0);
		return E_WRONG_SIZE;
	}

	/* Build RBAR: base address + access permissions + XN + shareability */
	region_def->_MPU_RBAR = (((uint32_t) base) & MPU_RBAR_BASE_Msk)
		| __MPU_v8m_flags[cls].rbar_flags;

	/* Build RLAR: limit address + attribute index + enable */
	region_def->_MPU_RLAR = (limit & MPU_RLAR_LIMIT)
		| ((__MPU_v8m_flags[cls].attr_idx << MPU_RLAR_ATTRINDX_LSB) & MPU_RLAR_ATTRINDX)
		| MPU_RLAR_ENABLE;

	return E_OK;
}

#endif

int mpu_clear_region(uint8_t region)
{
	MPU->RNR = ((region << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
	/* ARMv8M: clear enable bit in RLAR */
	MPU->RLAR &= ~MPU_RLAR_ENABLE;
#else
	/* ARMv6M/ARMv7M: clear enable bit in RASR */
	MPU->RASR &= ~MPU_RASR_ENABLE;
#endif
	return E_OK;
}

/** @} */

void os_memory_protection_start()
{
	mpu_set_region(OS_MPU_REGION_EXECUTABLE, (const void *) code_base(), code_size(), MPU_RX);
	mpu_enable();
}

void os_memory_protection_stop()
{
	mpu_disable();
}


int mpu_init_stack(int thread_id)
{
	const uint8_t thread_stack = os_threads[thread_id].stack_id;
    return mpu_set_region(OS_MPU_REGION_STACK, &os_stacks.stacks[thread_stack], sizeof(os_stacks.stacks[thread_stack]), MPU_RW);

}

bool mpu_check_bounds(const MPU_State * state, uint8_t region, uint32_t * address)
{
	// TODO: Make use of this
	(void) state;
	(void) region;
	(void) address;
	return false;
}

