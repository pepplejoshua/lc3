const std = @import("std");

// 2 ^ 16 memory locations = 65,536 memory locations
const MEMORY_MAX = 1 << 16;
// 65,536 memory locations of 16 bits each (addressable with a 16 bit integer)
// = 131,072 bits = 128 kilobytes (because 131072 / 1024 = 128)
var memory = [MEMORY_MAX]u16{};

// 10 total registers, each 16 bits wide:
// - 8 general purpose registers used for calculations (R0 - R7)
// - 1 program counter register (PC)
// - 1 condition flags register (COND)
const Register = enum {
    R_R0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND, // provides info about most recent calculation
    R_COUNT,
};

fn register2usize(reg: Register) usize {
    return @as(usize, @intFromEnum(reg));
}

// essentially, 10 registers but with Zig Flair
// the original code in C used the fact that enums are just integers under the hood
// here, it is more explicit: convert R_COUNT to an int, then convert it to usize (10),
// with R_R0 being 0 in this enum.
const registers = [register2usize(Register.R_COUNT)]u16{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// we will capture information about the most recent calculation
// by setting R_COND
const Flag = enum(u8) {
    Pos = 1 << 0, // P = 1
    Zero = 1 << 1, // Z = 2
    Neg = 1 << 2, // N = 4
};

// instruction opcodes
const Op = enum {
    Br, // Branch
    Add, // Add
    Load, // Load
    Store, // Store
    JumpSR, // Jump Register
    And, // Bitwise And

    LoadR, // Load Register
    StoreR, // Store Register
    RTI, // Unused

    Not, // Bitwise Not
    LoadI, // Load Indirect
    StoreI, // Store Indirect
    Jump, // Jump
    RES, // Reserved
    LoadEA, // load effective address
    Trap, // execute trap
};

pub fn main() void {
    std.debug.print("{}\n", .{registers.len});
}
