#define SYSCALLS \
    SYSCALL(Log, sys_log) \
    SYSCALL(Mmap, sys_mmap) \
    SYSCALL(Open, sys_open) \
    SYSCALL(Read, sys_read) \
    SYSCALL(Seek, sys_seek) \
    SYSCALL(Close, sys_close) \
    SYSCALL(TcbSet, sys_tcb_set) \
    SYSCALL(Exit, sys_exit) \
    SYSCALL(Write, sys_write) \
    SYSCALL(FutexWait, sys_futex_wait) \
    SYSCALL(FutexWake, sys_futex_wake) \
    SYSCALL(GetCwd, sys_get_cwd) \
    SYSCALL(OpenAt, sys_openat) \
    SYSCALL(Stat, sys_stat) \
    SYSCALL(ReadEntries, sys_read_entries) \
    SYSCALL(Chdir, sys_chdir) \
    SYSCALL(DrawFromBuffer, sys_draw_from_buffer) \
    SYSCALL(Mkdir, sys_mkdir) \
    SYSCALL(ClockGet, sys_clock_get) \
    SYSCALL(Sleep, sys_sleep) \
    SYSCALL(GetKbEvent, sys_get_kb_event) \
    SYSCALL(CreateSocket, sys_socket) \
    SYSCALL(SendMsg, sys_send_msg) \
    SYSCALL(RecvMsg, sys_recv_msg) \
    SYSCALL(SetSocketOpt, sys_set_sock_opt) \

#define SYSCALL(code, handler) code, 
enum SyscallCode {
    SYSCALLS
    END
};
#undef SYSCALL

[[gnu::always_inline]] inline uint64_t _syscall(uint64_t code, uint64_t arg1 = 0, uint64_t arg2 = 0, 
        uint64_t arg3 = 0, uint64_t arg4 = 0, uint64_t arg5 = 0, uint64_t arg6 = 0) {
    uint64_t ret;
    asm volatile(
        "movq %[code], %%rdi\n"
        "movq %[arg1], %%rsi\n"
        "movq %[arg2], %%rdx\n"
        "movq %[arg3], %%rcx\n"
        "movq %[arg4], %%r8\n"
        "movq %[arg5], %%r9\n"
        "movq %[arg6], %%r10\n"
        "int $0xfe\n"
        "movq %%rax, %[ret]"
        : [ret] "=r"(ret)
        : [code] "r"(code), [arg1] "r"(arg1), [arg2] "r"(arg2), [arg3] "r"(arg3), 
          [arg4] "r"(arg4), [arg5] "r"(arg5), [arg6] "r"(arg6)
        : "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9", "%r10", "%rax"
    );
    return ret;
}

template <typename... T>
[[gnu::always_inline]] inline uint64_t syscall(uint64_t code, T... args) {
    return _syscall(code, (uint64_t)(args)...);
}
