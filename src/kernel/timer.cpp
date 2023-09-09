#include <kernel/timer.hpp>
#include <kernel/apic.hpp>
#include <kernel/scheduler.hpp>

namespace Timer {

// UNIX time on boot, in milliseconds
static uint64_t real_time;

static uint32_t apic_calibrated_ticks;
static uint32_t cycles_per_ms;

void init() {
    // The response from the bootloader is given in seconds
    real_time = time_request.response->boot_time * 1000;
}

namespace Pit {
    static uint64_t pit_ticks = 0;
    static uint64_t tsc_initial = 0;

    // Called every time the PIT triggers an interrupt. Because we are only using the PIT for
    // calibration, this will only be called once.
    void handler([[gnu::unused]] Interrupt::Registers* regs) {
        if (tsc_initial == 0) {
            // Setup the initial value of the TSC that will be used to calculate how much cycles 
            // passed
            tsc_initial = __rdtsc();
        }

        pit_ticks++;

        // After 100 ms, the calibration is done
        if (pit_ticks == 100) {
            // How many ticks and cycles passed in 10ms
            uint32_t time_elapsed = UINT32_MAX -
                Apic::Local::read_reg(Apic::Local::Register::TimerCount);
            uint64_t cycles_elapsed = __rdtsc() - tsc_initial;

            // Disable PIT timer interrupts by masking the redirection
            Apic::Io::redirect_pin(0x14, TIMER_VECT, 1);

            // Calculate how many ticks passed in the lapic timer in 1 ms. This will be our initial
            // count for the timer so that it triggers interrupts every 1 ms.
            apic_calibrated_ticks = time_elapsed / 100;
            Apic::Local::write_reg(Apic::Local::Register::TimerInitCount, apic_calibrated_ticks);
            // Calculate how many cycles passed in 1 ms
            cycles_per_ms = cycles_elapsed / 100; 

            // We use a divisor of 2
            Apic::Local::write_reg(Apic::Local::Register::TimerDivisor, 0);

            // Redirect the local apic timer interrupt and make it periodic
            Apic::Local::write_reg(Apic::Local::Register::TimerLvt, TIMER_VECT | (1 << 17));
            Interrupt::set_handler(TIMER_VECT, Timer::Lapic::handler);
        }
    }
}

namespace Lapic {
    // Lapic apic timer handler, called every ms
    void handler(Interrupt::Registers* regs) {
        real_time++;
        Scheduler::schedule(regs);
    }

    void stop() {
        //Apic::Local::write_reg(Apic::Local::Register::TimerInitCount, 0); 
        // Mask the timer
        Apic::Local::write_reg(Apic::Local::Register::TimerLvt, (1 << 16));
    }

    void resume() {
        Apic::Local::write_reg(Apic::Local::Register::TimerInitCount, apic_calibrated_ticks); 
        Apic::Local::write_reg(Apic::Local::Register::TimerLvt, TIMER_VECT | (1 << 17));
    }
}

uint64_t ms_to_cycles(uint64_t ms) {
    return cycles_per_ms * ms;
}

uint64_t get_real_time() {
    return real_time;
}

}
