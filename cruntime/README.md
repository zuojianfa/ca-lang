$ clang -v poly.o 
clang version 12.0.1
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/lib/llvm/12/bin
Selected GCC installation: /usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0
Candidate multilib: .;@m64
Candidate multilib: 32;@m32
Selected multilib: .;@m64
 "/usr/bin/x86_64-pc-linux-gnu-ld" --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o a.out /usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0/../../../../lib64/crt1.o /usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0/../../../../lib64/crti.o /usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0/crtbegin.o -L/usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0 -L/usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0/../../../../lib64 -L/usr/lib/llvm/12/bin/../lib64 -L/lib/../lib64 -L/usr/lib/../lib64 -L/usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0/../../../../x86_64-pc-linux-gnu/lib -L/usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0/../../.. -L/lib -L/usr/lib poly.o -lgcc --as-needed -lgcc_s --no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed /usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0/crtend.o /usr/lib/gcc/x86_64-pc-linux-gnu/10.3.0/../../../../lib64/crtn.o
 
 
The c runtime library is copied from upper command line, just for convienient, so copied them here.

crt1.o crti.o crtn.o: coming from glibc, version: sys-libs/glibc-2.33-r1
crtbegin.o  crtend.o: coming from gcc, version: sys-devel/gcc-10.3.0-r1

For the license see: glibc, gcc

