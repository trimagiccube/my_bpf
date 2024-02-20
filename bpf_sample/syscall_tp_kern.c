/* Copyright (c) 2017 Facebook
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 */
#include <uapi/linux/bpf.h>
#include "bpf_helpers.h"

struct syscalls_enter_open_args {
	unsigned long long unused;
	long syscall_nr;
	long filename_ptr;
	long flags;
	long mode;
};

struct syscalls_exit_open_args {
	unsigned long long unused;
	long syscall_nr;
	long ret;
};

/*see /sys/kernel/debug/tracing/events/syscalls/sys_enter_fchownat/format*/
struct syscalls_enter_fchownat_args {
	unsigned long long unused;
	long syscall_nr;
	long dfd;
	long filename_ptr;
	long user;
	long group;
	long flag;
};

struct bpf_map_def SEC("maps") exit_open_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(u32),
	.value_size = sizeof(u32),
	.max_entries = 1,
};

struct bpf_map_def SEC("maps") enter_open_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(u32),
	.value_size = sizeof(u32),
	.max_entries = 1,
};
struct bpf_map_def SEC("maps") enter_fchownat_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(u32),
	.value_size = sizeof(u32),
	.max_entries = 1,
};

static __always_inline void count(void *map)
{
	u32 key = 0;
	u32 *value, init_val = 1;

	value = bpf_map_lookup_elem(map, &key);
	if (value)
		*value += 1;
	else
		bpf_map_update_elem(map, &key, &init_val, BPF_NOEXIST);
}
SEC("tracepoint/syscalls/sys_enter_open")
int trace_enter_open(struct syscalls_enter_open_args *ctx)
{
	count(&enter_open_map);
	return 0;
}

SEC("tracepoint/syscalls/sys_enter_openat")
int trace_enter_open_at(struct syscalls_enter_open_args *ctx)
{
	count(&enter_open_map);
	return 0;
}
SEC("tracepoint/syscalls/sys_exit_open")
int trace_enter_exit(struct syscalls_exit_open_args *ctx)
{
	count(&exit_open_map);
	return 0;
}

SEC("tracepoint/syscalls/sys_exit_openat")
int trace_enter_exit_at(struct syscalls_exit_open_args *ctx)
{
	count(&exit_open_map);
	return 0;
}

SEC("tracepoint/syscalls/sys_enter_fchownat")
int trace_enter_fchownat(struct syscalls_enter_fchownat_args *ctx)
{
	count(&enter_fchownat_map);
	return 0;
}
char _license[] SEC("license") = "GPL";
