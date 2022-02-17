#include <cmrx/shim/static.h>
#include <unistd.h>

static struct OS_process_definition_t * process_table = NULL;
unsigned process_count = 0;

static struct OS_thread_create_t * thread_table = NULL;
unsigned thread_count = 0;

void provide_process_table(struct OS_process_definition_t * table, unsigned count)
{
	process_table = table;
	process_count = count;
}

void provide_thread_table(struct OS_thread_create_t * table, unsigned count)
{
	thread_table = table;
	thread_count = count;
}

unsigned static_init_thread_count()
{
	return thread_count;
}

const struct OS_thread_create_t * const static_init_thread_table()
{
	return thread_table;
}

unsigned static_init_process_count()
{
	return process_count;
}

const struct OS_process_definition_t * const static_init_process_table()
{
	return process_table;
}

