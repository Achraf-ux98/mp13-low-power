# STM32MP13 LPLV-Stop2 Entry From Zephyr PM

Enter STM32MP13 LPLV-Stop2 from a Zephyr application by jumping back into the
ST FSBL-A code that still lives in SYSRAM, then resume Zephyr normally after wake.

## Two-Program Model

The system runs as two separate programs:

1. **FSBL-A** — loaded by BootROM into SYSRAM. Owns the real LPLV-Stop2 sequence
   (PMIC, DDR self-refresh, BSEC, final low-power entry) and the wake path.
2. **Zephyr** — loaded by FSBL-A into DDR and run as the main application.

Zephyr does not implement the low-power sequence itself. It prepares the wake
source, jumps into FSBL's low-power entry, and FSBL later jumps back to an
address Zephyr chose. Handoff both ways goes through RTC backup registers,
because those live in the always-on domain and survive the power-down (DDR
content and CPU registers do not).

## Two Ways To Trigger Low-Power Entry

There are two ways to enter the low-power sequence, and this project has a
commit for each on the `results` branch.

### 1. Manual entry from `main()` — proven

Commit: `lowPower Entry from main fully functionning`.

Here the low-power jump is called directly from the application, not through
Zephyr PM. The flow is:

```text
Zephyr main -> wakeWrap.c -> FSBL lp_enter -> LPLV-Stop2 -> FSBL lp_exit -> Zephyr main
```

The fallback after wake is `main` itself, so the program simply lands back in a
loop. That makes it easy to validate the whole chain — prepare wake source, jump
to FSBL, enter LPLV-Stop2, wake, and return. This part works end to end.

### 2. Automatic entry through Zephyr PM — in progress

An application task runs; when it is delayed or no runnable task remains, the
scheduler runs the idle thread (`zephyr/kernel/idle.c`). If PM is enabled, the
idle path calls `pm_system_suspend()` → `pm_state_set()`, which is where the
STM32MP13 backend takes over and jumps to FSBL.

This is harder because Zephyr expects `pm_state_set()` to *return normally* and
then continue its automatic resume sequence — but FSBL is a separate program, so
the jump back is not a normal C return. The rest of this document is about making
that resume work. Everything below describes this PM path.

## Files

| File | Role |
| --- | --- |
| `lowpowerentry/src/main.c` | Test app: RTC init, wake button, LED workload |
| `lowpowerentry/src/wakeWrap.c` | Maps FSBL SYSRAM and branches into FSBL entry |
| `zephyr/soc/st/stm32/stm32mp13x/power.c` | Zephyr PM backend: entry + wake landing |
| `zephyr/subsys/pm/pm.c` | Generic Zephyr PM core (`pm_system_suspend`) |
| `zephyr/kernel/idle.c` | Generic idle thread that calls system PM |

## Backup Register Contract

| Register | Owner | Meaning |
| --- | --- | --- |
| `RTC_BKP_DR0` | Zephyr → FSBL | Address FSBL jumps to on wake (`fsbl_zephyr_resume_entry`) |
| `RTC_BKP_DR2` | FSBL | FSBL low-power entry address (`lp_enter`), read in `wakeWrap.c` |
| `RTC_BKP_DR4` | Zephyr | Saved stack pointer of `pm_state_set` |
| `RTC_BKP_DR6/7/8` | Zephyr | Saved `r4` / `r5` / `r6` from `pm_state_set`'s frame |
| `RTC_BKP_DR9` | Zephyr | Saved return address (`lr`) of `pm_state_set` |

The gaps (`DR1`, `DR3`, `DR5`) are deliberately left unused so the layout stays
easy to extend and never collides with a register FSBL might touch. Confirming
the full set of FSBL-owned registers is still a remaining task (see *Status*).

FSBL also saves its own stack pointer in a retained SYSRAM slot (`0x5c00a110`),
found by tracing FSBL's own save/restore of its stack across the power-down.
Zephyr must not overwrite that slot.

## Control Flow

```text
idle thread (idle.c)
  -> pm_system_suspend()            (pm.c)
       -> pm_state_set()            (power.c)  ---- jumps to FSBL ---->  LPLV-Stop2
                                                                              |
                                                                            wake
  <-- fsbl_zephyr_resume_entry() <--- FSBL bx to RTC_BKP_DR0 <-----------------
       (rebuilds pm_state_set's return, bx lr)
  -> pm_state_set() returns into pm_system_suspend()
  -> pm_system_resume(), scheduler unlock, normal execution continues
```

## Entry Side: `pm_state_set()`

Called by the PM core when idle selects a low-power state.

1. **Re-entry guard.** `fsbl_resume_pending` is a one-shot flag. First pass jumps
   to FSBL; the pass after resume returns immediately so we do not jump twice.
2. **Program the return address.** Write `fsbl_zephyr_resume_entry` into
   `RTC_BKP_DR0`. This is where FSBL will `bx` after wake.
3. **Snapshot the return context** (see next section) into backup registers.
4. **Unlock IRQs** with `arch_irq_unlock(0)`. The idle thread enters PM with
   interrupts locked; FSBL's low-power/wake path needs them behaving like the
   proven bare-`main()` experiment. Kept local to `power.c` instead of editing
   generic `idle.c`.
5. **Jump to FSBL** via `wokeup()` (`wakeWrap.c`), which maps FSBL SYSRAM and
   `bx`es to the entry read from `RTC_BKP_DR2`. Execution leaves Zephyr here.
   (The name `wokeup()` is misleading \u2014 this function *enters* low power; it is
   not the wake handler. That role belongs to `fsbl_zephyr_resume_entry()`.)

## The Register / Stack Trick

`pm_state_set` never returns normally — it jumps into FSBL, a separate program
that powers the chip down. On wake, FSBL does a raw `bx` to `RTC_BKP_DR0`. There
is no call-stack linkage left: LR chain, live registers, and saved frame are all
gone. So the wake landing must **manually fake the CPU state that a normal return
from `pm_state_set` would produce**, then branch to the instruction after the
original call.

### Why r4/r5/r6/lr

The compiler prologue for `pm_state_set` is:

```asm
pm_state_set:
    push {r4, r5, r6, lr}
```

Under AAPCS:

- `r4`–`r11` are **callee-saved**. The caller (`pm_system_suspend`) holds live
  values in `r4/r5/r6` and expects them intact after the call, so the prologue
  saves them.
- `lr` holds the **return address** that `bl pm_state_set` produced. It is pushed
  because `pm_state_set` makes its own `bl` calls that would overwrite `lr`.

So right after the prologue the stack holds exactly the context needed to return:

```text
sp+0  -> saved r4        (caller's r4)
sp+4  -> saved r5        (caller's r5)
sp+8  -> saved r6        (caller's r6)
sp+12 -> saved lr        (return address into pm_system_suspend)
```

That is 4 words = 16 bytes.

### Capture (in `pm_state_set`)

```c
__asm__ volatile("mov %0, sp" : "=r"(fsbl_resume_sp));
fsbl_resume_frame = (uintptr_t *)fsbl_resume_sp;
fsbl_resume_lr = fsbl_resume_frame[3];   /* sp+12 = real return address */
```

`frame[0..2]` are the saved r4/r5/r6; `frame[3]` is the return address. All five
values (frame + `sp`) are written to backup registers so they survive power-down.

`sp` is captured **after** the prologue push, so it points at `saved r4`. That is
what makes `saved_sp + 16` in the reconstruction land exactly at the caller's
frame — the offset is load-bearing and depends on this capture point. `frame[3]`
is the pushed `lr` verbatim, so its bit 0 (the ARM/Thumb selector) is preserved
for free; the reconstruction never masks it.

### Reconstruction (in `fsbl_zephyr_resume_entry`)

```asm
mov r4, %0          ; restore caller's r4
mov r5, %1          ; restore caller's r5
mov r6, %2          ; restore caller's r6
mov lr, %3          ; restore return address into pm_system_suspend
add sp, %4, #16     ; sp = saved_sp + 16 -> "pop" the 4-word frame
bx  lr              ; branch back into pm_system_suspend
```

- `mov r4/r5/r6` — put the caller's callee-saved values back; `pm_system_suspend`
  reads these after the call.
- `mov lr` — load the verified return address (`pm_system_suspend`, right after
  `bl pm_state_set`).
- `add sp, saved_sp, #16` — simulate the epilogue's `pop {r4,r5,r6,pc}`, which
  advances sp by 16. We already have the values, so we just move sp past the
  frame; `pm_system_suspend` keeps using the stack below this point.
- `bx lr` — branch-and-exchange, so the ARM/Thumb state comes from bit 0 of the
  saved `lr`. From the PM core's view, `pm_state_set` just returned cleanly.

### Why r7/r8 are not saved

`pm_system_suspend` pushes `{r4,r5,r6,r7,r8,lr}` in **its own** prologue. Those
live on its own DDR stack frame and are reloaded by its own final `pop`. We only
reconstruct what `pm_state_set`'s frame held, so r7/r8 are not our concern.

## Wake Side: `fsbl_zephyr_resume_entry()`

FSBL jumps here via `RTC_BKP_DR0`. It reads the saved context back from the
backup registers, then runs the reconstruction above to return into the frozen
`pm_system_suspend`.

## Wake Source

Configured in `main.c` before entering low power:

```c
HAL_PWR_EnableWakeUpPinIT(PWR_WAKEUP_PIN1);
HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_LOW);
IRQ_Enable(MPU_WAKEUP_PIN_IRQn);
```

Without a valid wake source, FSBL enters low power but the board will not wake
through the intended path.

## SYSRAM Mapping Before The Jump

FSBL code and its stack live in SYSRAM (~`0x2FFE0000`–`0x30000000`). With the MMU
enabled under Zephyr, that region must be mapped executable/RW before branching,
or FSBL faults as soon as it runs or touches its own stack:

```c
arch_mem_map((void *)FSBL_SYSRAM_BASE, FSBL_SYSRAM_BASE, FSBL_SYSRAM_SIZE,
             K_MEM_ARM_NORMAL_NC | K_MEM_PERM_RW | K_MEM_PERM_EXEC);
```

An earlier missing mapping showed up as a Data Abort around `0x2fffea80` (the FSBL
SYSRAM stack). Mapping the full region fixed it. The final branch is a raw
`bx`, not a C call:

```c
__asm__ volatile("bx %0" : : "r"(fsbl_entry) : "memory");
```

## Known Wake-Side Fault: `ACTLR.SMP`

The FSBL LPLV-Stop2 wake path **clears `ACTLR.SMP` (bit 6)**. On Cortex-A7 that
disables the L1 exclusive monitor, so the first `LDREX`/`STREX` after resume —
i.e. any `atomic_*`, which the scheduler uses immediately — faults with a
**Synchronous External Abort** on a DDR address.

Symptom seen: the reconstruction correctly returns into the PM core (the
`PM: returning to PM core ...` print appears), then execution dies on the first
atomic (`atomic_add` in `z_setup_new_thread`). Confirmed by reading `ACTLR` on
resume: `SMP=0`.

Fix: re-assert `ACTLR.SMP` as the **very first thing** in
`fsbl_zephyr_resume_entry` — before the frame reconstruction, since `bx lr` hands
control straight into PM-core code that runs an atomic almost immediately:

```c
uint32_t actlr;
__asm__ volatile("mrc p15, 0, %0, c1, c0, 1" : "=r"(actlr)); /* ACTLR */
actlr |= (1U << 6);                                          /* SMP */
__asm__ volatile("mcr p15, 0, %0, c1, c0, 1" : : "r"(actlr));
__asm__ volatile("isb");
```

## Proven Baseline: Direct `main()` Entry

Entering low power directly from the application `main()` path is fully proven
(result: `lowPower Entry from main fully functionning`, see "Two Ways To Trigger
Low-Power Entry" above). Moving entry into Zephyr PM is what requires the
register/stack reconstruction above.

## Expected Debug Prints

Entry:

```text
PM: entered pm_state_set(state=1, substate=0)
PM: jumping to FSBL low-power entry
PM: BKP_DR2 jump target = 0x2ffe0191
step: fsbl entrypoint loaded
step: fsbl sysram mapped
step: jumping to fsbl entry 0x2ffe0191
```

Wake return:

```text
PM: FSBL returned to Zephyr resume entry
PM: returning to PM core SP=... LR=...
```

With the `ACTLR.SMP` fix in place, execution then continues past this point into
the PM core resume and back to the application.

## Status

Working / proven:

- Read FSBL entry from `RTC_BKP_DR2`, map SYSRAM, branch to FSBL `lp_enter`.
- FSBL enters LPLV-Stop2 and wakes on the button.
- FSBL jumps back to `fsbl_zephyr_resume_entry`.
- Frame reconstruction returns into `pm_system_suspend`.
- `ACTLR.SMP` re-assert clears the post-wake atomic Data Abort.

Remaining:

- Fully continuous resume through the PM core back to the normal application
  under all conditions.
- Replace the scraped-frame reconstruction with a fixed-ABI handoff stub.
- Confirm all RTC backup registers used by Zephyr are free of FSBL use.
- Remove debug prints once the resume path is stable.
