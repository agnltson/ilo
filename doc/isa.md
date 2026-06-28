# Belt CPU Architecture

---

# 1. Architectural Overview

The Belt CPU is a 32-bit architecture based on a 32-entry belt structure.

- The belt contains 32 values of 32 bits.
- `belt[0]` is the most recently produced value.
- Each new produced value shifts the belt.
- All instructions are 32 bits wide.
- All unused fields in any instruction MUST be zero.

The architecture contains:

- A 32-bit Program Counter (PC)
- A flat 32-bit address space
- A hardware frame stack (opaque to software)

Memory is byte-addressed.

---

# 2. Instruction Encoding

The least significant bit (bit 0) determines instruction format:

- bit[0] = 1 → Type-1 (Belt/Scratchpad indexed)
- bit[0] = 0 → Type-0 (Immediate / Control)

---

## 2.1 Type-1 Format (Indexed Operations)

```
31              16 15        11 10         6 5        0
+-----------------+------------+------------+----------+
|  Reserved (16)  | OperandB   | OperandA   | Opcode   |
|                 |   (5)      |   (5)      |  (6)     |
+-----------------+------------+------------+----------+
```

Constraints:
- OperandA and OperandB are belt indexes (0–31) or scratchpad indexes (0–31).
- Result is inserted at belt[0].
- bit[0] of Opcode MUST be 1.
- Reserved bits MUST be 0.

---

## 2.2 Type-0 Format (Immediate / Control)

```
31                             6 5        0
+-------------------------------+----------+
|   Immediate (26 bits)         | Opcode   |
+-------------------------------+----------+
```

Constraints:
- Immediate is a signed 26-bit value (two's complement).
- bit[0] of Opcode MUST be 0.

For control-flow instructions:
- Immediate is a signed PC-relative offset.
- Offset is expressed in instruction words.
- Target = PC + 1 + Immediate.

---

# 3. Execution Model

Unless specified otherwise:

- PC increments by 1 after each instruction.
- For control-flow instructions, PC is modified as defined.
- All results are inserted at belt[0].
- Belt behaves as a shift register of 32 entries.

---

## 3.1 Frame Model

A **frame** is the execution context of a function. Each frame owns:

- A private **belt** (32 × 32-bit values).
- A private **scratchpad** (32 × 32-bit registers, hardware, not memory-mapped).

Frames are managed by a hardware **frame stack**, opaque to software. Software cannot directly read or write the frame stack.

On `call`:
1. The current frame (belt, scratchpad, PC+1) is pushed onto the frame stack.
2. A new frame is created:
   - belt is a full copy of the caller's belt.
   - scratchpad is initialized to zero.
3. PC jumps to the target.

On `ret`:
1. `retval = belt[0]` of the current frame is saved.
2. The previous frame is popped from the frame stack (restoring belt, scratchpad, and saved PC).
3. `retval` is inserted at `belt[0]` of the restored frame, shifting it.
4. PC is restored to the saved return address.

If a function produces no meaningful return value, the caller ignores `belt[0]` by convention. The belt shift still occurs.

---

# 4. Type-1 Instructions (bit0 = 1)

## 4.1 Arithmetic

All arithmetic is 32-bit signed.

| Instruction | Opcode |
|-------------|--------|
| add | 000001 |
| sub | 000011 |
| mul | 000101 |
| div | 000111 |

Semantics:

```
add A,B:
belt[0] = belt[A] + belt[B]
```

Division by zero behavior: implementation-defined.

---

## 4.2 Binary

| Instruction | Opcode |
|-------------|--------|
| and | 001001 |
| or  | 001011 |
| xor | 001101 |
| sll | 001111 |
| srl | 010001 |
| sra | 010011 |

Semantics:

```
sll A,B:
belt[0] = belt[A] << (belt[B] & 0x1F)
```

`srl` is logical shift right.
`sra` is arithmetic shift right.

---

## 4.3 Compare

Result:
- 1 if true
- 0 if false

| Instruction | Opcode |
|-------------|--------|
| eq | 010101 |
| lt | 010111 |

Semantics:

```
eq A,B:
belt[0] = (belt[A] == belt[B]) ? 1 : 0
```

`lt` is signed comparison.

---

## 4.4 Scratchpad

The scratchpad is a set of 32 private hardware registers per frame.
It is initialized to zero on frame creation and is not accessible from other frames.

### put

| Opcode |
|--------|
| 011001 |

Semantics:

```
scratchpad[A] = belt[B]
```

---

### pick

| Opcode |
|--------|
| 011011 |

Semantics:

```
belt[0] = scratchpad[A]
```

OperandB MUST be zero.

---

## 4.5 Memory

Memory is a flat array of 32-bit words.

### load

| Instruction | Opcode |
|-------------|--------|
| load8  | 100001 |
| load16 | 100101 |
| load32 | 101011 |

Semantics:

```
belt[0] = memory[belt[A]]
```

OperandB MUST be zero.

---

### store

| Instruction | Opcode |
|-------------|--------|
| store8  | 100011 |
| store16 | 100111 |
| store32 | 101011 |

Semantics:

```
memory[belt[A]] = belt[B]
```

---

# 5. Type-0 Instructions (bit0 = 0)

## 5.1 Utils

| Instruction | Opcode |
|-------------|--------|
| nop | 000000 |

Semantics:
No effect.

---

## 5.2 Immediate Construction

### immh

| Opcode |
|--------|
| 000010 |

Semantics:

```
belt[0] = Immediate << 12
```

Only the upper 20 bits of the Immediate field are significant.

---

### imml

| Opcode |
|--------|
| 000100 |

Semantics:

```
belt[0] = Immediate & 0xFFF
```

Only the lower 12 bits are significant.

Full 32-bit constants are constructed using:

```
immh
imml
or 0, 1
```

---

## 5.3 Control Flow

Offsets are PC-relative.

Target computation:

```
target = PC + 1 + offset
```

---

### jmp

| Opcode |
|--------|
| 000110 |

Semantics:

```
PC = target
```

---

### jmpif

| Opcode |
|--------|
| 001000 |

Semantics:

```
if belt[0] != 0:
    PC = target
```

---

### call

| Opcode |
|--------|
| 001010 |

Semantics:

```
frame_stack.push(current_frame.belt, current_frame.scratchpad, PC + 1)
new_frame.belt      = current_frame.belt   ; full copy
new_frame.scratchpad = 0                   ; zeroed
PC = target
```

See section 3.1 for the full frame model.

---

### ret

| Opcode |
|--------|
| 001100 |

Semantics:

```
retval = belt[0]
(saved_belt, saved_scratchpad, saved_PC) = frame_stack.pop()
belt      = saved_belt
scratchpad = saved_scratchpad
PC        = saved_PC
belt[0]   = retval    ; shifts the restored belt
```

If the function produces no meaningful return value, the caller ignores `belt[0]` by convention.

---

## 5.4 System Control

### halt

| Opcode |
|--------|
| 111110 |

Semantics:
Execution stops.

---

# 6. Architectural Guarantees

- All instructions are 32 bits.
- All unused fields MUST be zero.
- Type is determined by bit[0].
- Memory is byte addressed.
- Memory uses little-endian byte order.
- Arithmetic is two's complement signed.
- The scratchpad is hardware-local to each frame and not memory-mapped.
- The frame stack is hardware-managed and opaque to software.

---

# End of Specification
