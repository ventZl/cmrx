#include <cmrx/mpu.h>
#include <cmrx/defines.h>
#include <libopencm3/cm3/mpu.h>
#include <conf/kernel.h>
#include <cmrx/intrinsics.h>
#include <cmrx/assert.h>
#include <libopencm3/cm3/scb.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/syscall.h>
#include <cmrx/intrinsics.h>

#include <stdio.h>

void hard_fault_handler(void)
{
	uint32_t status = SCB_CFSR;
	if ((status & SCB_CFSR_IACCVIOL) || (status & SCB_CFSR_DACCVIOL))
	{
		if (status & SCB_CFSR_MMARVALID)
		{
			uint32_t addr = SCB_MMFAR;
			printf("Segmentation fault at address 0x%08X in thread %d\n", (uint32_t) addr, os_get_current_thread());
		}
	}
	ASSERT(0);
}

void mem_manage_handler(void)
{
	ASSERT(0);
}

static inline uint8_t log_2(uint32_t num)
{
	return __builtin_ctz(num);
}

void mpu_enable()
{
	MPU_CTRL |= MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE;
}

void mpu_disable()
{
	MPU_CTRL &= ~(MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE);
}

int mpu_store(MPU_State * state)
{
	if (state == NULL)
		return E_INVALID_ADDRESS;

	for (int q = 0; q < MPU_STATE_SIZE; ++q)
	{
		MPU_RNR = ((q << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		(*state)[q]._MPU_RBAR = MPU_RBAR;
		(*state)[q]._MPU_RASR = MPU_RASR;
	}

	return E_OK;
}

int mpu_restore(const MPU_State * state)
{
	if (state == NULL)
		return E_INVALID_ADDRESS;

	for (int q = 0; q < MPU_STATE_SIZE; ++q)
	{
		MPU_RNR = ((q << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
		MPU_RBAR = (*state)[q]._MPU_RBAR;
		MPU_RASR = (*state)[q]._MPU_RASR;
	}

	return E_OK;
}

int mpu_set_region(uint8_t region, const void * base, uint32_t size, uint32_t flags)
{
	uint8_t regszbits = ((sizeof(uint32_t)*8) - 1) - __builtin_clz(size);
	uint32_t subregions = 0xFF;

	// In below code we don't need to take special care about
	// bits above regszbits, because we know they are all zeroes.

	if ((size & ((1 << regszbits) - 1)) != 0)
	{
		ASSERT(0);
		// for now simply ignore this case
		return E_MISALIGNED;

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

		uint32_t base_misalignment = ((uint32_t) base & ~((1 << regszbits) - 1)) >> (regszbits - 3);
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
		subregions = 
			(1 << ((size_subregions + base_misalignment + 1) - 1)) 
			& ~((1 << base_misalignment) - 1);
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
	MPU_RBAR = ((region << MPU_RBAR_REGION_LSB) & MPU_RBAR_REGION)
		| (((uint32_t) base) & MPU_RBAR_ADDR) | MPU_RBAR_VALID;

	MPU_RASR = ((regszbits << MPU_RASR_SIZE_LSB) & MPU_RASR_SIZE)
		| ((subregions << MPU_RASR_SRD_LSB) & MPU_RASR_SRD)
		| (flags & (MPU_RASR_ATTR_AP | MPU_RASR_ATTR_XN))
		| MPU_RASR_ATTR_C
		| MPU_RASR_ENABLE;

	__ISB();
	__DSB();
	return E_OK;

}

int mpu_clear_region(uint8_t region)
{
	MPU_RNR = ((region << MPU_RNR_REGION_LSB) & MPU_RNR_REGION);
	MPU_RASR &= ~MPU_RASR_ENABLE;
	return E_OK;
}
