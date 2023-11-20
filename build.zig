const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});

    const optimize = b.standardOptimizeOption(.{});
    //const aegis_dep = b.dependency("aegis", .{
    //    .target = target,
    //});
    const lib = b.addStaticLibrary(.{
        .name = "picotls",
        //.root_source_file = .{ .path = "src/aegis.zig" },
        .target = target,
        .optimize = optimize,
    });
    lib.addCSourceFiles(.{
        .files = &picotls_core_files,
        .flags = &.{},
    });
    lib.addCSourceFiles(.{
        .files = &picotls_minicrypto_files,
        .flags = &.{},
    });
    lib.addCSourceFiles(.{
        .files = &minicrypto_library_files,
        .flags = &.{},
    });
    inline for (&include_paths) |path| {
        lib.addIncludePath(.{ .path = path });
    }
    //lib.defineCMacro("PTLS_HAVE_AEGIS", "1");
    //lib.linkLibrary(aegis_dep.artifact("libaegis"));
    lib.linkLibC();
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
    "deps/cifra/src/ext",
    "deps/cifra/src",
    "deps/micro-ecc",
    "picotlsvs/picotls",
    "include",
};

const picotls_core_files = [_][]const u8{
    "lib/hpke.c",
    "lib/picotls.c",
    "lib/pembase64.c",
};

const picotls_minicrypto_files = [_][]const u8{
    "lib/cifra.c",
    "lib/cifra/x25519.c",
    "lib/cifra/chacha20.c",
    "lib/cifra/aes128.c",
    "lib/cifra/aes256.c",
    "lib/cifra/random.c",
    "lib/minicrypto-pem.c",
    "lib/uecc.c",
    "lib/asn1.c",
    "lib/ffx.c",
    //"lib/cifra/libaegis.c",
};

const minicrypto_library_files = [_][]const u8{
    "deps/micro-ecc/uECC.c",
    "deps/cifra/src/aes.c",
    "deps/cifra/src/blockwise.c",
    "deps/cifra/src/chacha20.c",
    "deps/cifra/src/chash.c",
    "deps/cifra/src/curve25519.c",
    "deps/cifra/src/drbg.c",
    "deps/cifra/src/hmac.c",
    "deps/cifra/src/gcm.c",
    "deps/cifra/src/gf128.c",
    "deps/cifra/src/modes.c",
    "deps/cifra/src/poly1305.c",
    "deps/cifra/src/sha256.c",
    "deps/cifra/src/sha512.c",
};
