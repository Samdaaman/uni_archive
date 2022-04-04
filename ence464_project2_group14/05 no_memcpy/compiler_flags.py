import os
import sys
sys.path.append('..')
from benchmark import time_it

flags = """-O0
-O1
-O2
-O3
-Ofast
-Og
-Os
-O3 -Ofast
-fauto-inc-dec 
-fbranch-count-reg 
-fcombine-stack-adjustments 
-fcompare-elim 
-fcprop-registers 
-fdce 
-fdefer-pop 
-fdelayed-branch 
-fdse 
-fforward-propagate 
-fguess-branch-probability 
-fif-conversion 
-fif-conversion2 
-finline-functions-called-once 
-fipa-profile 
-fipa-pure-const 
-fipa-reference 
-fipa-reference-addressable 
-fmerge-constants 
-fmove-loop-invariants 
-fomit-frame-pointer 
-freorder-blocks 
-fshrink-wrap 
-fshrink-wrap-separate 
-fsplit-wide-types 
-fssa-backprop 
-fssa-phiopt 
-ftree-bit-ccp 
-ftree-ccp 
-ftree-ch 
-ftree-coalesce-vars 
-ftree-copy-prop 
-ftree-dce 
-ftree-dominator-opts 
-ftree-dse 
-ftree-forwprop 
-ftree-fre 
-ftree-phiprop 
-ftree-pta 
-ftree-scev-cprop 
-ftree-sink 
-ftree-slsr 
-ftree-sra 
-ftree-ter 
-funit-at-a-time
-falign-functions
-falign-jumps 
-falign-labels
-falign-loops 
-fcaller-saves 
-fcode-hoisting 
-fcrossjumping 
-fcse-follow-jumps
-fcse-skip-blocks 
-fdelete-null-pointer-checks 
-fdevirtualize
-fdevirtualize-speculatively 
-fexpensive-optimizations 
-fgcse
-fgcse-lm  
-fhoist-adjacent-loads 
-finline-functions 
-finline-small-functions 
-findirect-inlining 
-fipa-bit-cp
-fipa-cp
-fipa-icf 
-fipa-ra
-fipa-sra
-fipa-vrp 
-fisolate-erroneous-paths-dereference 
-flra-remat 
-foptimize-sibling-calls 
-foptimize-strlen 
-fpartial-inlining 
-fpeephole2 
-freorder-blocks-algorithm=stc 
-freorder-blocks-and-partition
-freorder-functions 
-frerun-cse-after-loop  
-fschedule-insns
-fschedule-insns2 
-fsched-interblock
-fsched-spec 
-fstore-merging 
-fstrict-aliasing 
-fthread-jumps 
-ftree-builtin-call-dce 
-ftree-pre 
-ftree-switch-conversion
-ftree-tail-merge 
-ftree-vrp"""

N = 201
print(f'Testing compiler flags for {N}x{N}x{N}')

flags = flags.split('\n')
for i, flag in enumerate(flags):
    if os.system(f'gcc -o main.o {flag} -DPRINT_TIME main.c -lpthread') != 0:
        print(f'{i} {flag} compile failed')

    times = [f'{time_it(N)}' for _ in range(10)]
    print(f'{i+1} / {len(flags)}', file=sys.stderr)
    print(','.join([f'{i}'] + times))
    
