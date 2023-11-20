const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});
    const mbedtls_dep = b.dependency("mbedtls", .{
        .target = target,
        .optimize = optimize,
    });
    const lib = b.addStaticLibrary(.{
        .name = "picotls",
        //.root_source_file = .{ .path = "src/aegis.zig" },
        .target = target,
        .optimize = optimize,
    });
    lib.addCSourceFiles(.{
        .files = &picotls_mbedtls_files,
        .flags = &.{},
    });
    inline for (&include_paths) |path| {
        lib.addIncludePath(.{ .path = path });
    }
    lib.defineCMacro("PTLS_HAVE_MBEDTLS", "1");
    //lib.linkLibrary(aegis_dep.artifact("libaegis"));
    lib.linkLibC();
    lib.linkLibrary(mbedtls_dep.artifact("mbedtls"));
    b.installArtifact(lib);

    const main_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    const run_main_tests = b.addRunArtifact(main_tests);
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&run_main_tests.step);
}

const include_paths = [_][]const u8{
    "include",
};

const picotls_mbedtls_files = [_][]const u8{
    "src/mbedtls.c",
};
