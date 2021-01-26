#!/usr/bin/env python3

# parses output of bpu_mem_dtlb_store attack to check for TLB misses
# during recovery. If the attack succeeds, the element indexed by the
# the secret value (42) will hit in the TLB during recovery, while
# the other elements will miss.

import re

from argparse import ArgumentParser
from pprint import pprint

SECRET_VALUE = 42

def getSlotAddresses( output ):
    slots = {}
    for l in output:
        try:
            result = re.match( r"array2\[(\d+) \* STEP_SIZE\] @ (0x[0-9a-fA-F]+)",
                               l.strip() ).groups()
        except:
            continue

        idx, addr = result
        slots[addr] = int(idx)

    return slots

def collectMisses( output, transmission_slots ):
    tlb_misses = {}
    for i in range(256):
        tlb_misses[i] = False

    for l in output:
        if  "Handling a TLB miss for address" in l:
            res = re.match( r".*address (0x[0-9a-fA-F]+)", l.strip() )
            if res is not None:
                addr, = res.groups()
                if addr in transmission_slots:
                    idx = transmission_slots[addr]
                    tlb_misses[idx] = True

    return tlb_misses

def main():
    parser = ArgumentParser(description=
      'Parsing logs of TLB attack')

    parser.add_argument('--log',
                        help='raw log of attack',
                        default="out.txt", nargs='?')
    parser.add_argument('--simulated-times-file',
                        help='raw log of attack',
                        default="times.csv", nargs='?')

    args = parser.parse_args()

    with open( args.log ) as output_file:
        output = output_file.readlines()

    transmission_slots = getSlotAddresses( output )
    tlb_misses = collectMisses( output, transmission_slots )

    for i in range(1,256):
        print( "tlb_misses[%d] = %s" % (i, tlb_misses[i] ) )

    if tlb_misses[SECRET_VALUE]:
        print("Attack failed. Could not determine the secret ({})".format(SECRET_VALUE))
    else:
        print("Attack succeeded. Secret correctly guessed as {}".format(SECRET_VALUE))


if __name__ == '__main__':
    main()
