/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include "ansi_c_internal_additions.h"

#include <util/config.h>

const char gcc_builtin_headers_generic[]=
"# 1 \"gcc_builtin_headers_generic.h\"\n"
#include "gcc_builtin_headers_generic.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_math[]=
"# 1 \"gcc_builtin_headers_math.h\"\n"
#include "gcc_builtin_headers_math.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_mem_string[]=
"# 1 \"gcc_builtin_headers_mem_string.h\"\n"
#include "gcc_builtin_headers_mem_string.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_omp[]=
"# 1 \"gcc_builtin_headers_omp.h\"\n"
#include "gcc_builtin_headers_omp.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_tm[]=
"# 1 \"gcc_builtin_headers_tm.h\"\n"
#include "gcc_builtin_headers_tm.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_ubsan[]=
"# 1 \"gcc_builtin_headers_ubsan.h\"\n"
#include "gcc_builtin_headers_ubsan.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_ia32[]=
"# 1 \"gcc_builtin_headers_ia32.h\"\n"
#include "gcc_builtin_headers_ia32.inc"
; // NOLINT(whitespace/semicolon)
const char gcc_builtin_headers_ia32_2[]=
#include "gcc_builtin_headers_ia32-2.inc"
; // NOLINT(whitespace/semicolon)
const char gcc_builtin_headers_ia32_3[]=
#include "gcc_builtin_headers_ia32-3.inc"
; // NOLINT(whitespace/semicolon)
const char gcc_builtin_headers_ia32_4[]=
#include "gcc_builtin_headers_ia32-4.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_alpha[]=
"# 1 \"gcc_builtin_headers_alpha.h\"\n"
#include "gcc_builtin_headers_alpha.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_arm[]=
"# 1 \"gcc_builtin_headers_arm.h\"\n"
#include "gcc_builtin_headers_arm.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_mips[]=
"# 1 \"gcc_builtin_headers_mips.h\"\n"
#include "gcc_builtin_headers_mips.inc"
; // NOLINT(whitespace/semicolon)

const char gcc_builtin_headers_power[]=
"# 1 \"gcc_builtin_headers_power.h\"\n"
#include "gcc_builtin_headers_power.inc"
; // NOLINT(whitespace/semicolon)

const char arm_builtin_headers[]=
"# 1 \"arm_builtin_headers.h\"\n"
#include "arm_builtin_headers.inc"
; // NOLINT(whitespace/semicolon)

const char cw_builtin_headers[]=
"# 1 \"cw_builtin_headers.h\"\n"
#include "cw_builtin_headers.inc"
; // NOLINT(whitespace/semicolon)

const char clang_builtin_headers[]=
"# 1 \"clang_builtin_headers.h\"\n"
#include "clang_builtin_headers.inc"
; // NOLINT(whitespace/semicolon)

static std::string architecture_string(const std::string &value, const char *s)
{
  return std::string("const char *__CPROVER_architecture_")+
         std::string(s)+
         "=\""+value+"\";\n";
}

static std::string architecture_string(int value, const char *s)
{
  return std::string("const int __CPROVER_architecture_")+
         std::string(s)+
         "="+std::to_string(value)+";\n";
}

void ansi_c_internal_additions(std::string &code)
{
  code+=
    "# 1 \"<built-in-additions>\"\n"
    "typedef __typeof__(sizeof(int)) __CPROVER_size_t;\n"
    "void __CPROVER_assume(__CPROVER_bool assumption);\n"
    "void __VERIFIER_assume(__CPROVER_bool assumption);\n"
    // NOLINTNEXTLINE(whitespace/line_length)
    "void __CPROVER_assert(__CPROVER_bool assertion, const char *description);\n"
    // NOLINTNEXTLINE(whitespace/line_length)
    "void __CPROVER_precondition(__CPROVER_bool precondition, const char *description);\n"
    "void __CPROVER_havoc_object(void *);\n"
    "__CPROVER_bool __CPROVER_equal();\n"
    "__CPROVER_bool __CPROVER_same_object(const void *, const void *);\n"
    "__CPROVER_bool __CPROVER_invalid_pointer(const void *);\n"
    "__CPROVER_bool __CPROVER_is_zero_string(const void *);\n"
    "__CPROVER_size_t __CPROVER_zero_string_length(const void *);\n"
    "__CPROVER_size_t __CPROVER_buffer_size(const void *);\n"

    "__CPROVER_bool __CPROVER_get_flag(const void *, const char *);\n"
    "void __CPROVER_set_must(const void *, const char *);\n"
    "void __CPROVER_clear_must(const void *, const char *);\n"
    "void __CPROVER_set_may(const void *, const char *);\n"
    "void __CPROVER_clear_may(const void *, const char *);\n"
    "void __CPROVER_cleanup(const void *, const void *);\n"
    "__CPROVER_bool __CPROVER_get_must(const void *, const char *);\n"
    "__CPROVER_bool __CPROVER_get_may(const void *, const char *);\n"

    "const unsigned __CPROVER_constant_infinity_uint;\n"
    "typedef void __CPROVER_integer;\n"
    "typedef void __CPROVER_rational;\n"
    "void __CPROVER_initialize(void);\n"
    "void __CPROVER_input(const char *id, ...);\n"
    "void __CPROVER_output(const char *id, ...);\n"
    "void __CPROVER_cover(__CPROVER_bool condition);\n"

    // concurrency-related
    "void __CPROVER_atomic_begin();\n"
    "void __CPROVER_atomic_end();\n"
    "void __CPROVER_fence(const char *kind, ...);\n"
    "__CPROVER_thread_local unsigned long __CPROVER_thread_id=0;\n"
    // NOLINTNEXTLINE(whitespace/line_length)
    "__CPROVER_bool __CPROVER_threads_exited[__CPROVER_constant_infinity_uint];\n"
    "unsigned long __CPROVER_next_thread_id=0;\n"

    // traces
    "void CBMC_trace(int lvl, const char *event, ...);\n"

    // pointers
    "unsigned __CPROVER_POINTER_OBJECT(const void *p);\n"
    "signed __CPROVER_POINTER_OFFSET(const void *p);\n"
    "__CPROVER_bool __CPROVER_DYNAMIC_OBJECT(const void *p);\n"
    "extern unsigned char __CPROVER_memory[__CPROVER_constant_infinity_uint];\n"
    // NOLINTNEXTLINE(whitespace/line_length)
    "void __CPROVER_allocated_memory(__CPROVER_size_t address, __CPROVER_size_t extent);\n"

    // malloc
    "void *__CPROVER_allocate(__CPROVER_size_t size, __CPROVER_bool zero);\n"
    "const void *__CPROVER_deallocated=0;\n"
    "const void *__CPROVER_dead_object=0;\n"
    "const void *__CPROVER_malloc_object=0;\n"
    "__CPROVER_size_t __CPROVER_malloc_size;\n"
    "__CPROVER_bool __CPROVER_malloc_is_new_array=0;\n" // for C++
    "const void *__CPROVER_memory_leak=0;\n"

    // this is ANSI-C
    // NOLINTNEXTLINE(whitespace/line_length)
    "extern __CPROVER_thread_local const char __func__[__CPROVER_constant_infinity_uint];\n"

    // this is GCC
    // NOLINTNEXTLINE(whitespace/line_length)
    "extern __CPROVER_thread_local const char __FUNCTION__[__CPROVER_constant_infinity_uint];\n"
    // NOLINTNEXTLINE(whitespace/line_length)
    "extern __CPROVER_thread_local const char __PRETTY_FUNCTION__[__CPROVER_constant_infinity_uint];\n"

    // float stuff
    "__CPROVER_bool __CPROVER_isnanf(float f);\n"
    "__CPROVER_bool __CPROVER_isnand(double f);\n"
    "__CPROVER_bool __CPROVER_isnanld(long double f);\n"
    "__CPROVER_bool __CPROVER_isfinitef(float f);\n"
    "__CPROVER_bool __CPROVER_isfinited(double f);\n"
    "__CPROVER_bool __CPROVER_isfiniteld(long double f);\n"
    "__CPROVER_bool __CPROVER_isinff(float f);\n"
    "__CPROVER_bool __CPROVER_isinfd(double f);\n"
    "__CPROVER_bool __CPROVER_isinfld(long double f);\n"
    "__CPROVER_bool __CPROVER_isnormalf(float f);\n"
    "__CPROVER_bool __CPROVER_isnormald(double f);\n"
    "__CPROVER_bool __CPROVER_isnormalld(long double f);\n"
    "__CPROVER_bool __CPROVER_signf(float f);\n"
    "__CPROVER_bool __CPROVER_signd(double f);\n"
    "__CPROVER_bool __CPROVER_signld(long double f);\n"
    "double __CPROVER_inf(void);\n"
    "float __CPROVER_inff(void);\n"
    "long double __CPROVER_infl(void);\n"
    "int __CPROVER_thread_local __CPROVER_rounding_mode="+
      std::to_string(config.ansi_c.rounding_mode)+";\n"
    "int __CPROVER_isgreaterf(float f, float g);\n"
    "int __CPROVER_isgreaterd(double f, double g);\n"
    "int __CPROVER_isgreaterequalf(float f, float g);\n"
    "int __CPROVER_isgreaterequald(double f, double g);\n"
    "int __CPROVER_islessf(float f, float g);\n"
    "int __CPROVER_islessd(double f, double g);\n"
    "int __CPROVER_islessequalf(float f, float g);\n"
    "int __CPROVER_islessequald(double f, double g);\n"
    "int __CPROVER_islessgreaterf(float f, float g);\n"
    "int __CPROVER_islessgreaterd(double f, double g);\n"
    "int __CPROVER_isunorderedf(float f, float g);\n"
    "int __CPROVER_isunorderedd(double f, double g);\n"

    // absolute value
    "int __CPROVER_abs(int x);\n"
    "long int __CPROVER_labs(long int x);\n"
    "long int __CPROVER_llabs(long long int x);\n"
    "double __CPROVER_fabs(double x);\n"
    "long double __CPROVER_fabsl(long double x);\n"
    "float __CPROVER_fabsf(float x);\n"

    // arrays
    // overwrite all of *dest (possibly using nondet values), even
    // if *src is smaller
    "void __CPROVER_array_copy(const void *dest, const void *src);\n"
    // replace at most size-of-*src bytes in *dest
    "void __CPROVER_array_replace(const void *dest, const void *src);\n"
    "void __CPROVER_array_set(const void *dest, ...);\n"

    // k-induction
    "void __CPROVER_k_induction_hint(unsigned min, unsigned max, "
      "unsigned step, unsigned loop_free);\n"

    // format string-related
    "int __CPROVER_scanf(const char *, ...);\n"

    // pipes, write, read, close
    "struct __CPROVER_pipet {\n"
    "  _Bool widowed;\n"
    "  char data[4];\n"
    "  short next_avail;\n"
    "  short next_unread;\n"
    "};\n"
    // NOLINTNEXTLINE(whitespace/line_length)
    "extern struct __CPROVER_pipet __CPROVER_pipes[__CPROVER_constant_infinity_uint];\n"
    // offset to make sure we don't collide with other fds
    "extern const int __CPROVER_pipe_offset;\n"
    "unsigned __CPROVER_pipe_count=0;\n"

    "\n";

  // GCC junk stuff, also for CLANG and ARM
  if(config.ansi_c.mode==configt::ansi_ct::flavourt::GCC ||
     config.ansi_c.mode==configt::ansi_ct::flavourt::APPLE ||
     config.ansi_c.mode==configt::ansi_ct::flavourt::ARM)
  {
    code+=gcc_builtin_headers_generic;
    code+=gcc_builtin_headers_math;
    code+=gcc_builtin_headers_mem_string;
    code+=gcc_builtin_headers_omp;
    code+=gcc_builtin_headers_tm;
    code+=gcc_builtin_headers_ubsan;
    code+=clang_builtin_headers;

    // there are many more, e.g., look at
    // https://developer.apple.com/library/mac/#documentation/developertools/gcc-4.0.1/gcc/Target-Builtins.html

    if(config.ansi_c.arch=="i386" ||
       config.ansi_c.arch=="x86_64" ||
       config.ansi_c.arch=="x32")
    {
      if(config.ansi_c.mode==configt::ansi_ct::flavourt::APPLE)
        code+="typedef double __float128;\n"; // clang doesn't do __float128

      code+=gcc_builtin_headers_ia32;
      code+=gcc_builtin_headers_ia32_2;
      code+=gcc_builtin_headers_ia32_3;
      code+=gcc_builtin_headers_ia32_4;
    }
    else if(config.ansi_c.arch=="arm64" ||
            config.ansi_c.arch=="armel" ||
            config.ansi_c.arch=="armhf" ||
            config.ansi_c.arch=="arm")
    {
      code+=gcc_builtin_headers_arm;
    }
    else if(config.ansi_c.arch=="alpha")
    {
      code+=gcc_builtin_headers_alpha;
    }
    else if(config.ansi_c.arch=="mips64el" ||
            config.ansi_c.arch=="mipsn32el" ||
            config.ansi_c.arch=="mipsel" ||
            config.ansi_c.arch=="mips64" ||
            config.ansi_c.arch=="mipsn32" ||
            config.ansi_c.arch=="mips")
    {
      code+=gcc_builtin_headers_mips;
    }
    else if(config.ansi_c.arch=="powerpc" ||
            config.ansi_c.arch=="ppc64" ||
            config.ansi_c.arch=="ppc64le")
    {
      code+=gcc_builtin_headers_power;
    }

    // On 64-bit systems, gcc has typedefs
    // __int128_t und __uint128_t -- but not on 32 bit!
    if(config.ansi_c.long_int_width>=64)
    {
      code+="typedef signed __int128 __int128_t;\n"
            "typedef unsigned __int128 __uint128_t;\n";
    }
  }

  // this is Visual C/C++ only
  if(config.ansi_c.os==configt::ansi_ct::ost::OS_WIN)
    code+="int __noop();\n"
          "int __assume(int);\n";

  // ARM stuff
  if(config.ansi_c.mode==configt::ansi_ct::flavourt::ARM)
    code+=arm_builtin_headers;

  // CW stuff
  if(config.ansi_c.mode==configt::ansi_ct::flavourt::CODEWARRIOR)
    code+=cw_builtin_headers;

  // Architecture strings
  ansi_c_architecture_strings(code);
}

void ansi_c_architecture_strings(std::string &code)
{
  // The following are CPROVER-specific.
  // They allow identifying the architectural settings used
  // at compile time from a goto-binary.

  code+="# 1 \"<builtin-architecture-strings>\"\n";

  code+=architecture_string(config.ansi_c.int_width, "int_width");
  code+=architecture_string(config.ansi_c.int_width, "word_size"); // old
  code+=architecture_string(config.ansi_c.long_int_width, "long_int_width");
  code+=architecture_string(config.ansi_c.bool_width, "bool_width");
  code+=architecture_string(config.ansi_c.char_width, "char_width");
  code+=architecture_string(config.ansi_c.short_int_width, "short_int_width");
  code+=architecture_string(config.ansi_c.long_long_int_width, "long_long_int_width"); // NOLINT(whitespace/line_length)
  code+=architecture_string(config.ansi_c.pointer_width, "pointer_width");
  code+=architecture_string(config.ansi_c.single_width, "single_width");
  code+=architecture_string(config.ansi_c.double_width, "double_width");
  code+=architecture_string(config.ansi_c.long_double_width, "long_double_width"); // NOLINT(whitespace/line_length)
  code+=architecture_string(config.ansi_c.wchar_t_width, "wchar_t_width");
  code+=architecture_string(config.ansi_c.char_is_unsigned, "char_is_unsigned");
  code+=architecture_string(config.ansi_c.wchar_t_is_unsigned, "wchar_t_is_unsigned"); // NOLINT(whitespace/line_length)
  code+=architecture_string(config.ansi_c.use_fixed_for_float, "fixed_for_float"); // NOLINT(whitespace/line_length)
  code+=architecture_string(config.ansi_c.alignment, "alignment");
  code+=architecture_string(config.ansi_c.memory_operand_size, "memory_operand_size"); // NOLINT(whitespace/line_length)
  code+=architecture_string(static_cast<int>(config.ansi_c.endianness), "endianness"); // NOLINT(whitespace/line_length)
  code+=architecture_string(id2string(config.ansi_c.arch), "arch");
  code+=architecture_string(configt::ansi_ct::os_to_string(config.ansi_c.os), "os"); // NOLINT(whitespace/line_length)
  code+=architecture_string(config.ansi_c.NULL_is_zero, "NULL_is_zero");
}
