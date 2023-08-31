# Notes on using rdtsc(p) instruction: zen 4 ryzen mobile processors (R7 7840H) and intel 12th gen alder lake mobile processor (i7 12700H) as examples

## How to properly use the rdtsc(p) instruction

On x86 platforms the `rdtsc` and `rdtscp` instructions gave us the ability to access a internal 64-bit hardware time stamp counter TSC(Time Stamp Counter) that is reset to zero at boot time and since then increments at a certain frequency (ideally at the same rate as the processor clock speed), which can be very useful for conducting microbenchmarks.

Here's some good references to learn about the `rdtsc` and the `rdtscp` instructions:

- [Stackoverflow thread](https://stackoverflow.com/questions/59759596/is-there-any-difference-in-between-rdtsc-lfence-rdtsc-and-rdtsc-rdtscp)
- [Intel's guide on benchmarking code with rdtsc(p)](https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf)
- [rdtscp reference](https://www.felixcloutier.com/x86/rdtscp)
- [rdtsc reference](https://c9x.me/x86/html/file_module_x86_id_278.html)

### First glance

For measuring the execution time of `work`, the ideal case would be like the following:
```python
leading_code()
startcycle = rdtsc()

measured_code()

endcycle = rdtsc()
following_code()
```
However things just won't work out this easy. The main problem is with out-of-order and multi-issue execution. The plain `rdtsc` instruction does NOT give any serializing guarantess. This means that:

- At the time of `startcycle=rdtsc` execution, the `leading code` part could have unfinished instructions that contaminants the `measured_code`
- At the time of `startcycle=rdtsc` actually read the hardware counter, the `measured_code` could have already started its execution.
- At the time of `endcycl=rdtsc` execution, the `measured_code` could be unfinished.
- At the time of `endcycl=rdtsc` actually read the hardware counter, the `following_code` could have started its execution, taking up hardware resources and thus causing structural hazard and the delays the `endcycl=rdtsc`'s reading

So how do we solve these problems? It turns out that some *serializing* instructions has to be used.

### The `rdtscp` ,`lfence` and `cpuid`

For solving the above problems, some more or less *serializing* instructions has to be used. We have some candidates here:

1. The `rdtscp` instruction, which is available all recent X86 platforms. This instruction could be used to read the hardware counter like the `rdtsc` instruction, and while it **is not a serializing instruction, it does wait until all previous instructions have executed and all previous loads are globally visible.** Besides however, **it does not wait for previous stores to be globally visible, and subsequent instructions may begin execution before the read operation is performed.** This means that if we modify the above code to the following version:

    ```python
    leading_code()
    startcycle = rdtscp()

    measured_code()

    endcycle = rdtscp()
    following_code()
    ```
    Two of the above problems is solved that
    -  the `leading_code` must have finished when the `startcycle = rdtscp()` begins to execute.
    -  the `measured_code` must have finished when the `endcycle = rdtscp()` begins to execute.

    While two other problems remains to be solved:

    - At the time of `startcycle=rdtscp` actually read the hardware counter, the `measured_code` could still have already started its execution.
    - At the time of `endcycl=rdtscp` actually read the hardware counter, the `following_code` could still have started its execution, taking up hardware resources and thus causing structural hazard and the delays the `endcycl=rdtscp`'s reading

    Now we need some instructions that **really** does some serializing. The first option is to use `cpuid`.

2. The `cpuid` instruction, which is available on all X86 platforms, is a true serializing instruction that **guarantees any modifications to flags, registers, and memory for previous instructions are completed before the next instruction is fetched and executed.** This means that if we modify the above code to the following version:

    ```python
    leading_code()
    startcycle = rdtscp()
    cpuid()

    measured_code()

    endcycle = rdtscp()
    cpuid()
    following_code()
    ```
    This way all the problems are solved. However we must notice that the execution time of first `cpuid` is included in the final timing result. This system error is inevitable and should be substracted from the final result. Besides, as [this] suggests, the use of `cpuid` instruction might severely hinders the performance. Turns out that the `lfence` instruction is a better choice.


3. The `lfence` instruction, which **performs a serializing operation on all load-from-memory instructions that were issued prior the LFENCE instruction. Specifically, LFENCE does not execute until all prior instructions have completed locally, and no later instruction begins execution until LFENCE completes.** The `lfence` instruction is not a true serializing instruction in that it only guarantees that the **LFENCE does not execute until all prior instructions have completed locally**. The key point is the adverb **locally** which is explained [here](https://stackoverflow.com/questions/37452772/x86-64-usage-of-lfence/37469559#37469559). Basically speaking `lfence` fits our needs here. However be ware that this **local serialization** behaviour is just guaranteed on intel processors. On earlier AMD processors the `lfence` only does a plain load fence, but for the recent enough ryzen processors `lfence` behaves the same as the intel ones, see [this](https://stackoverflow.com/questions/12631856/difference-between-rdtscp-rdtsc-memory-and-cpuid-rdtsc) for more details.

    So now our final code is like the below: 
    ```python
    leading_code()
    startcycle = rdtscp()
    lfence()

    measured_code()

    endcycle = rdtscp()
    lfence()
    following_code()
    ```

    Again, `lfence` introduces some inevitable system error that has to be substracted. Besides, its worth noting that a `lfence;rdtsc` sequence is roughly equavalent to `rdtscp` according to [this](https://stackoverflow.com/questions/59759596/is-there-any-difference-in-between-rdtsc-lfence-rdtsc-and-rdtsc-rdtscp).





## Implementations, more details and note that rdtscp's behaviour is different between AMD(7840H) and intel(12700H) platforms

### Implementation

With the above knowledges, we could implement a clock reading routine using inline assembly as the following:

```cpp
inline volatile void clock(uint32_t& clk_hi, uint32_t& clk_lo){
    asm volatile(
        "rdtscp\n\t"
        "lfence"
        : "=d"(clk_hi), "=a"(clk_lo)
        :
        : "%rcx"
    );
}
```



- resolution
- rdtscp;rdtscp: out of order

## The incrementing frequency of the TSC 

It is often desirable to interpret clock cycle count to real world time. To do so, given a clock driven counter, we would like to know the frequency of the clock. The increment frequency is usually the same as the base clock of the processor, on ryzen R7 7840H, this is about 3.8GHz (~100MHz * 38), however its always good to confirm this. AFAIK there is two ways to get this frequency.

#### 1. Get TSC frequency with the help of Linux

Very luckily Linux uses TSC as a major clock source and it performs a calibration at system boot time. Here `dmesg` log could help us confirm this calibration did happen:

```
$ sudo dmesg | grep -Ii tsc
[    0.000000] tsc: Fast TSC calibration using PIT
[    0.000000] tsc: Detected 3793.051 MHz processor
[    0.058695] clocksource: tsc-early: mask: 0xffffffffffffffff max_cycles: 0x6d59656ea52, max_idle_ns: 881590428463 ns
[    0.562150] clocksource: Switched to clocksource tsc-early
[    1.584321] tsc: Refined TSC clocksource calibration: 3792.875 MHz
[    1.584339] clocksource: tsc: mask: 0xffffffffffffffff max_cycles: 0x6d581b92771, max_idle_ns: 881590605997 ns
[    1.584392] clocksource: Switched to clocksource tsc
...
```
Then our problem is how to we read this calibration result. In fact Linux kernel has an exported symbol `tsc_khz` defined in [tsc.c](https://github.com/torvalds/linux/blob/master/arch/x86/kernel/tsc.c) however it is not exposed to userspace. Stackoverflow user maxschlepzig summarized many ways to access this symbol in [this stackoverflow thread](https://stackoverflow.com/questions/35123379/getting-tsc-rate-from-x86-kernel). I personally feel it is most reasonable and convenient to use a kernel module. 

https://github.com/trailofbits/tsc_freq_khz

#### 2. Performing a rather flawed calibration

It is also possible to get the tsc frequency by conducting a calibration. The idea is simple: with some wall clock provided by the system, we record start time with tsc clock, and end time with tsc clock, then divide the time difference by the clock difference to get the tsc cycle time.


## Constant TSC

#### 1. Constant TSC capability
It's natural to notice that to correctly interpret clock cycle count to real world time the frequency of tsc increment should be constant, or at least it should behave as if it is contant. On modern processors with `constant_tsc` and `nonstop_tsc` flags, the tsc always increment at a constant frequency no matter of the state of the processor such as the frequency scaling state. These flags could be checked with `cat /proc/cpuinfo | grep tsc`. 

#### 2. Pitfall of constant tsc
With a constant tsc, a tsc clock difference reading can always be safely interpreted to real world time difference. For instance, with a constant tsc calibrated to 3.8GHz, if two consecutive reading to the tsc gives a clock difference of 38, it is always true that the time interval between the two measurement was 10ns. However it is worth noting that even with a constant tsc, for an clock-accurate microbench result, it is still needed to fix the clock speed of the CPU to its base clock. This is because if not, with constant tsc the processor clock and the tsc clock is essentially running asynchronously. For instance, if the processor's running at 1900MHz while the tsc has a constant freq of 3800MHz, for some operation that takes 5 cycles to finish, a tsc-timed bench result would tell that it takes 10 clock cycles to complete. 

