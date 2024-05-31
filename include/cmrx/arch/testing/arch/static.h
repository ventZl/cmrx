#pragma once

unsigned static_init_thread_count();
const struct OS_thread_create_t * static_init_thread_table();

unsigned static_init_process_count();
const struct OS_process_definition_t * static_init_process_table();

