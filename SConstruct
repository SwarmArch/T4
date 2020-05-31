from __future__ import (absolute_import, division, print_function)

import distutils.spawn
import os
import subprocess

def sanitizeInput(userinput, allowables):
    if not (userinput in allowables):
        print("Error: expected one of [{}], found {}".format( \
                ', '.join(allowables), userinput))
        Exit(1)

runtime = ARGUMENTS.get('runtime', 'scc')
sanitizeInput(runtime, ['scc', 'scc_serial'])
AddOption('--allocator', dest='allocator',
          action='store', type='choice',
          default='simalloc',
          choices=['simalloc', 'tcmalloc', 'native'],
          help='Link with simalloc (for simulation), '
               'or link with tcmalloc or native (the system allocator, likely from glibc)')
AddOption('--flags', dest='flags', action='append',
          help='Additional flags to pass to the compiler')
AddOption('--llvm-flags', dest='llvm_flags', action='append',
          help='Additional flags to pass to LLVM')
AddOption('--scc-remarks', dest='scc_remarks', default=False, action='store_true',
          help='[scc] Print some remarks about SCC code transformations')
AddOption('--debug-fractalization', dest='debug_fractalization',
          default=False, action='store_true',
          help='[scc] Verbose output for automatic fractalization pass')
AddOption('--debug-loopexpansion', dest='debug_loopexpansion',
          default=False, action='store_true',
          help='[scc] Verbose output for loop expansion pass')
AddOption('--debug-swarmabi', dest='debug_swarmabi',
          default=False, action='store_true',
          help='[scc] Verbose output for lowering of Tapir-like IR to Swarm ABI')
AddOption('--debug-only', dest='debug_only',
          help='[scc] Comma-separated list of pass names with verbose output.')
AddOption('--debug-funcs', dest='debug_funcs',
          help='[scc] Comma-separated list of function names for which to print IR.')
AddOption('--no-insert-heartbeats', dest="no_insert_heartbeats",
          default=False, action='store_true',
          help='[scc] Disable instrumentation to track the amount of work performed')
AddOption('--no-tree-expansion',
          default=False, action='store_true',
          help='[scc] Disable all parallel tree expansion, creating chains only')
AddOption('--no-unbounded-tree-expansion',
          default=False, action='store_true',
          help='[scc] Disable progressive expansion of unbounded loops')
AddOption('--no-loop-coarsen',
          default=False, action='store_true',
          help='[scc] Disable loop task coarsening pass')
AddOption('--no-auto-hints', action='store_true',
          help='[scc] Disable spatial hint generation')
AddOption('--no-hints', action='store_true',
          help='[scc] Discard all hints (but keep SAMEHINT optimizations)')
AddOption('--no-optimize-live-ins', action='store_true',
          help='[scc] Disable live-in optimizations')
AddOption('--no-bundling', action='store_true',
          help='[scc] Disable bundling of allocas')
AddOption('--no-padding', action='store_true',
          help='[scc] Disable padding of bundled allocas')
AddOption('--no-privatization', action='store_true',
          help='[scc] Disable privatization')

env = Environment()
env.Replace(ENV = os.environ)

if not distutils.spawn.find_executable('clang'):
    print('Unable to find Clang.')
    Exit(1)
env['CC'] = 'clang'
env['CXX'] = 'clang++'

env['LLVMFLAGS'] = []  # SConscripts can access and append to LLVMFLAGS.
# To pass through Clang, each flag needs '-mllvm' in front of it.
env.Append(CPPFLAGS = "${sum([['-mllvm', f] for f in LLVMFLAGS], [])}")
env.Append(LLVMFLAGS = GetOption('llvm_flags'))

# Optionally turn on verbose debug output from selected passes.
env['LLVMDEBUGONLY'] = []  # SConscripts can access and append to this.
# Only LLVM builds with assertions enabled have the -debug-only option.
# As a hack to avoid build crashes when using precompiled LLVM/Clang,
# we must avoid passing -debug-only and pass some other innocuous flag.
env.Append(LLVMFLAGS = "${('-debug-only=' + ','.join(LLVMDEBUGONLY))" +
                       "  if LLVMDEBUGONLY else '-regalloc=default'}")
if GetOption('debug_only'):
    env.Append(LLVMDEBUGONLY=GetOption('debug_only').split(','))

if GetOption('debug_funcs'):
    env.Append(LLVMFLAGS=[
        # victory: Unfortunately LLVm's -print-before= doesn't work:
        # https://bugs.llvm.org/show_bug.cgi?id=30662
        '-print-before-all',
        Literal('-filter-print-funcs=' + GetOption('debug_funcs'))
    ])

if not GetOption('no_exec'):
    conf = env.Configure()
    checks = [
        conf.CheckCC(),
        # Does the following not work for scons 2.3.0?
        #conf.CheckCXX(),
    ]
    env = conf.Finish()
    if not all(checks): Exit(1)

includePath = os.path.join(Dir('#').abspath, 'include')
swarmIncludePath = os.path.join(Dir('#').abspath, 'swarm_runtime/include')
env.Append(CPPPATH = [swarmIncludePath, includePath])
env.Append(CXXFLAGS = ['-std=c++14'])
# Use C11 for C programs, but because -std=c11 has bad interactions with strdup
# (https://stackoverflow.com/a/20318225), we use -std=gnu11
env.Append(CFLAGS = ['-std=gnu11'])
runtimeName = runtime.upper() + '_RUNTIME'
env.Append(CPPDEFINES = {runtimeName : '1',
                         'SWARM_CACHE_LINE'             : '64',
                        })

env.Append(LIBS = ['pthread'])

env.Append(CPPFLAGS = [
        '-O3','-gdwarf-3',
        '-g', '-Wall',
        '-mno-avx', '-mno-avx2', '-mno-sse4.2',
        # Soft-disable x87, which isn't modeled correctly, and SSE is faster
        # anyhow. Entirely disabling x87 through -mno-80387 causes some
        # compilation errors as some calls return values in x87 regs.
        '-mfpmath=sse',
        # Various systems may enable -fstack-protector-strong by default, e.g.,
        # GCC in Ubuntu 14.10 and newer: https://perma.cc/V8PB-TAU4
        # That would add accesses to thread-local storage to get random canary
        # values that are placed on the stack.  We want to avoid these memory
        # accesses, which incur needless conflict checks:
        '-fno-stack-protector',
        ])

env.Append(CPPFLAGS = GetOption('flags'))

if 'scc' in runtime:
    clang_version = subprocess.check_output(['clang','-v'],
                                            stderr=subprocess.STDOUT,
                                            universal_newlines=True)
    print(clang_version)
    if ('swarm' not in clang_version.lower()
            and not clang_version.startswith('SCC')):
        print('ERROR! Using a non-SCC version of Clang.')
        Exit(1)

    # If you declare a C function without explicitly specifying the types of
    # it's arguments, the function is said to lack a prototype. For example:
    #   void foo();
    #   void bar(void);
    # In C++ these declarations would indicate both foo() and bar() take no
    # arguments, but in C, bar() has a prototype indicating it takes no
    # arguments and foo() has no prototype, so foo() could take any number of
    # arguments.  This causes pain for SCC, so give a warning:
    env.Append(CFLAGS = ['-Wstrict-prototypes'])

    env.Append(CPPFLAGS = ['-fswarm'])
    env.Append(CPPFLAGS = '-fno-exceptions')

    if GetOption('no_tree_expansion'):
        env.Append(LLVMFLAGS = ['-swarm-chainexpandonly'])
    if GetOption('no_unbounded_tree_expansion'):
        env.Append(LLVMFLAGS = ['-swarm-disableunboundedtrees'])
    if GetOption('no_loop_coarsen'):
        env.Append(LLVMFLAGS = ['-swarm-disableloopcoarsen'])
    if GetOption('no_auto_hints'):
        env.Append(LLVMFLAGS=['-swarm-disablehints'])
    if GetOption('no_hints'):
        env.Append(LLVMFLAGS = ['-swarm-disableallhints'])
    if GetOption('no_optimize_live_ins'):
        env.Append(LLVMFLAGS=['-swarm-disableshrinkinputs', '-swarm-disablesharedenv'])
    if GetOption('no_bundling'):
        env.Append(LLVMFLAGS=['-swarm-disablebundling'])
    if GetOption('no_padding'):
        env.Append(LLVMFLAGS=['-swarm-disablepadding'])
    if GetOption('no_privatization'):
        env.Append(LLVMFLAGS=['-swarm-disableprivatization'])

    if runtime == 'scc_serial':
        if GetOption('allocator') == 'simalloc':
            print('You probably meant to specify `--allocator=native` or `--allocator=tcmalloc`')
            Exit(1)

        env.Append(LLVMFLAGS=['-swarm-abi=serial'])

        # If an application has a separate SCC version of its code,
        # the scc_serial runtime should alse use that SCC version.
        env.Append(CPPDEFINES = {'SCC_RUNTIME': 2})

    # Optionally turn on optimization remarks
    env['REMARK_PASSES'] = '(' + ')|('.join([
            'parallelizable-copy',
            'profitability',
            'bundling',
            'fractalization',
            'swarm-reductions',
            'loop-coarsen',
            'swarm-loop-expansion',
            ]) + ')'
    env['REMARK_PASSES_OPT'] = (
            # Enable remarks if explicitly requested.
            "$REMARK_PASSES" if GetOption('scc_remarks')
            # Also enable remarks if any debug output is requested.
            else "${REMARK_PASSES if LLVMDEBUGONLY else 'none'}")
    env.Append(CPPFLAGS = ["$(",
                           "'-Rpass=$REMARK_PASSES_OPT'",
                           "'-Rpass-missed=$REMARK_PASSES_OPT'",
                           "'-Rpass-analysis=$REMARK_PASSES_OPT'",
                           "$)",
                          ])

    # More convenient options for SCons to enable verbose debug output
    # from SCC-specific passes.  If you misspell the SCons flag,
    # scons will give you an informative error.
    if GetOption('debug_fractalization'):
        env.Append(LLVMDEBUGONLY=['fractalization'])
    if GetOption('debug_loopexpansion'):
        env.Append(LLVMDEBUGONLY=['swarm-loop-expansion'])
    if GetOption('debug_swarmabi'):
        env.Append(LLVMDEBUGONLY=['lower2swarm', 'swarmabi'])

buildDir = os.path.join('./build', runtime)

AddOption('--clang', default=True, action='store_true')
libswarm, libsimalloc = SConscript('./swarm_runtime/lib/SConscript',
        variant_dir=os.path.join(buildDir, 'swarm_runtime', 'lib'),
        exports=['runtime'],
        duplicate=0)
if libswarm:
    env.Append(LIBS = libswarm)
    # Linking all programs with the C++ compiler driver ensures we link with
    # libstdc++ and toolchain-specific things (e.g., libgcc) consistently.
    # This is to ensure libswarm finds all its dependencies at link time.
    env['LINK'] = env['CXX']

if GetOption('allocator') == 'native':
    # Use system default malloc() implementation.
    pass
elif GetOption('allocator') == 'tcmalloc':
    # "If you'd rather link in a version of TCMalloc that does not include the
    # heap profiler and checker (perhaps to reduce binary size for a static
    # binary), you can link in libtcmalloc_minimal instead."
    # http://goog-perftools.sourceforge.net/doc/tcmalloc.html
    env.Prepend(LIBS = ['tcmalloc_minimal'])
else:
    # simalloc redirects allocation to the simulator.
    env.Prepend(LIBS = libsimalloc)
    # Linking all programs with the C++ compiler driver ensures we link with
    # libstdc++ and toolchain-specific things (e.g., libgcc) consistently.
    # This is to ensure libsimalloc finds all its dependencies at link time.
    env['LINK'] = env['CXX']

libsccrt = SConscript('./sccrt/SConscript',
                      variant_dir=os.path.join(buildDir, 'sccrt'),
                      exports=['runtime'],
                      duplicate=0)
env.Append(LIBS = libsccrt)
# Linking all programs with the C++ compiler driver ensures we link with
# libstdc++ and toolchain-specific things (e.g., libgcc) consistently.
# This is to ensure libsccrt finds all its dependencies at link time.
env['LINK'] = env['CXX']

# IMPORTANT: SPEC apps have a dot in the name, so would ignore PROGSUFFIX
env['PROGPREFIX'] = (
    '' if (GetOption('allocator') == 'simalloc') else
    '{}_'.format(GetOption('allocator'))
    )

benchmarks = [
              'microbenchmarks',
             ]

# "Note that VariantDir works most naturally with a subsidiary SConscript file."
# http://www.scons.org/doc/production/HTML/scons-man.html#f-VariantDir
VariantDir(buildDir, '.', duplicate=0)
SConscript(dirs=[os.path.join(buildDir, b) for b in benchmarks],
           exports=['env', 'runtime'])
