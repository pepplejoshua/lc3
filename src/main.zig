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
    R_COND,
    R_COUNT,
};

// essentially, 10 registers but with zig flair
// the original code in C used the fact that enums are just integers under the hood
// here, it is more explicit: convert R_COUNT to an int, then convert it to usize (10),
// with R_R0 being 0 in this enum.
const registers = [register2usize(Register.R_COUNT)]u16{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

fn register2usize(reg: Register) usize {
    return @as(usize, @intFromEnum(reg));
}

pub fn main() void {
    std.debug.print("{}\n", .{registers.len});
}
