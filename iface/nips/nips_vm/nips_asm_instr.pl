# NIPS (Dis)Asm - New Implementation of Promela Semantics (Dis-)Assembler
# Copyright (C) 2005: Stefan Schuermans <stefan@schuermans.info>
#                     Michael Weber <michaelw@i2.informatik.rwth-aachen.de>
#                     Lehrstuhl fuer Informatik II, RWTH Aachen
# Copyleft: GNU public license - http://www.gnu.org/copyleft/gpl.html

use strict;

# instructions
#  - array with fixed words (variable number of strings)
#    - e.g. instruction name
#    - e.g. instruction variant
#  - opcode
#  - array with parameters
#    - const1, const2, const4: constants with 1, 2, 4 bytes
#    - addr: relative address expressed by a label
#    - address: absolute address
#    - reg: a register r0 .. r7
sub get_instructions
{
  my ($cflow, $error) = (1, 1);
  my @instructions = (
    [ 0x00, [ "NOP" ] ],
    [ 0x01, [ "LDC", "!const4" ] ],
    [ 0x02, [ "LDV", "L", "1u" ] ],
    [ 0x03, [ "LDV", "L", "1s" ] ],
    [ 0x04, [ "LDV", "L", "2u" ] ],
    [ 0x05, [ "LDV", "L", "2s" ] ],
    [ 0x06, [ "LDV", "L", "4" ] ],
    [ 0x07, [ "LDV", "G", "1u" ] ],
    [ 0x08, [ "LDV", "G", "1s" ] ],
    [ 0x09, [ "LDV", "G", "2u" ] ],
    [ 0x0A, [ "LDV", "G", "2s" ] ],
    [ 0x0B, [ "LDV", "G", "4" ] ],
    [ 0x0C, [ "STV", "L", "1u" ] ],
    [ 0x0D, [ "STV", "L", "1s" ] ],
    [ 0x0E, [ "STV", "L", "2u" ] ],
    [ 0x0F, [ "STV", "L", "2s" ] ],
    [ 0x10, [ "STV", "L", "4" ] ],
    [ 0x11, [ "STV", "G", "1u" ] ],
    [ 0x12, [ "STV", "G", "1s" ] ],
    [ 0x13, [ "STV", "G", "2u" ] ],
    [ 0x14, [ "STV", "G", "2s" ] ],
    [ 0x15, [ "STV", "G", "4" ] ],
    [ 0x16, [ "TRUNC", "!const1" ] ],
    [ 0x18, [ "LDS", "timeout" ] ],
    [ 0x19, [ "LDS", "pid" ] ],
    [ 0x1A, [ "LDS", "nrpr" ] ],
    [ 0x1B, [ "LDS", "last" ] ],
    [ 0x1C, [ "LDS", "np" ] ],
    [ 0x20, [ "ADD" ] ],
    [ 0x21, [ "SUB" ] ],
    [ 0x22, [ "MUL" ] ],
    [ 0x23, [ "DIV" ] ],
    [ 0x24, [ "MOD" ] ],
    [ 0x25, [ "NEG" ] ],
    [ 0x26, [ "NOT" ] ],
    [ 0x27, [ "AND" ] ],
    [ 0x28, [ "OR" ] ],
    [ 0x29, [ "XOR" ] ],
    [ 0x2A, [ "SHL" ] ],
    [ 0x2B, [ "SHR" ] ],
    [ 0x2C, [ "EQ" ] ],
    [ 0x2D, [ "NEQ" ] ],
    [ 0x2E, [ "LT" ] ],
    [ 0x2F, [ "LTE" ] ],
    [ 0x30, [ "GT" ] ],
    [ 0x31, [ "GTE" ] ],
    [ 0x32, [ "BNOT" ] ],
    [ 0x33, [ "BAND" ] ],
    [ 0x34, [ "BOR" ] ],
    [ 0x40, [ "ICHK", "!const1" ], $error ],
    [ 0x41, [ "BCHK" ], $error ],
    [ 0x48, [ "JMP", "!addr" ], $cflow ],
    [ 0x49, [ "JMPZ", "!addr" ], $cflow ],
    [ 0x4A, [ "JMPNZ", "!addr" ], $cflow ],
    [ 0x4B, [ "LJMP", "!address" ], $cflow ],
    [ 0x50, [ "TOP", "!reg" ] ],
    [ 0x51, [ "POP", "!reg" ] ],
    [ 0x52, [ "PUSH", "!reg" ] ],
    [ 0x53, [ "POPX" ] ],
    [ 0x54, [ "INC", "!reg" ] ],
    [ 0x55, [ "DEC", "!reg" ] ],
    [ 0x56, [ "LOOP", "!reg", "!addr" ], $cflow ],
    [ 0x58, [ "CALL", "!addr" ], $cflow ],
    [ 0x59, [ "RET" ], $cflow ],
    [ 0x5A, [ "LCALL", "!address" ], $cflow ],
    [ 0x60, [ "CHNEW", "!const1", "!const1" ] ],
    [ 0x61, [ "CHMAX" ] ],
    [ 0x62, [ "CHLEN" ] ],
    [ 0x63, [ "CHFREE" ] ],
    [ 0x64, [ "CHADD" ] ],
    [ 0x65, [ "CHSET" ] ],
    [ 0x66, [ "CHGET" ] ],
    [ 0x67, [ "CHDEL" ] ],
    [ 0x68, [ "CHSORT" ] ],
    [ 0x6B, [ "CHROT" ] ],
    [ 0x6C, [ "CHSETO", "!const1" ] ],
    [ 0x6D, [ "CHGETO", "!const1" ] ],
    [ 0x70, [ "NDET", "!addr" ] ],
    [ 0x72, [ "ELSE", "!addr" ] ],
    [ 0x73, [ "UNLESS", "!addr" ] ],
    [ 0x74, [ "NEX" ], $cflow ],
    [ 0x75, [ "NEXZ" ], $cflow ],
    [ 0x76, [ "NEXNZ" ], $cflow ],
    [ 0x78, [ "STEP", "N", "!const1" ], $cflow ],
    [ 0x79, [ "STEP", "A", "!const1" ], $cflow ],
    [ 0x7A, [ "STEP", "I", "!const1" ], $cflow ],
    [ 0x7B, [ "STEP", "T", "!const1" ], $cflow ],
    [ 0x80, [ "RUN", "!const1", "!const1", "!addr" ] ],
    [ 0x81, [ "LRUN", "!const1", "!const1", "!address" ] ],
    [ 0x84, [ "GLOBSZ", "!const1" ] ],
    [ 0x85, [ "LOCSZ", "!const1" ] ],
    [ 0x86, [ "GLOBSZX", "!const2" ] ],
    [ 0x88, [ "FCLR" ] ],
    [ 0x89, [ "FGET", "!const1" ] ],
    [ 0x8A, [ "FSET", "!const1" ] ],
    [ 0x8C, [ "BGET", "!reg", "!const1" ] ],
    [ 0x8D, [ "BSET", "!reg", "!const1" ] ],
    [ 0x90, [ "PRINTS", "!const2" ] ],
    [ 0x91, [ "PRINTV", "!const1" ] ],
    [ 0x92, [ "LDVA", "L", "1u", "!const1" ] ],
    [ 0x93, [ "LDVA", "L", "1s", "!const1" ] ],
    [ 0x94, [ "LDVA", "L", "2u", "!const1" ] ],
    [ 0x95, [ "LDVA", "L", "2s", "!const1" ] ],
    [ 0x96, [ "LDVA", "L", "4", "!const1" ] ],
    [ 0x97, [ "LDVA", "G", "1u", "!const1" ] ],
    [ 0x98, [ "LDVA", "G", "1s", "!const1" ] ],
    [ 0x99, [ "LDVA", "G", "2u", "!const1" ] ],
    [ 0x9A, [ "LDVA", "G", "2s", "!const1" ] ],
    [ 0x9B, [ "LDVA", "G", "4", "!const1" ] ],
    [ 0x9C, [ "STVA", "L", "1u", "!const1" ] ],
    [ 0x9D, [ "STVA", "L", "1s", "!const1" ] ],
    [ 0x9E, [ "STVA", "L", "2u", "!const1" ] ],
    [ 0x9F, [ "STVA", "L", "2s", "!const1" ] ],
    [ 0xA0, [ "STVA", "L", "4", "!const1" ] ],
    [ 0xA1, [ "STVA", "G", "1u", "!const1" ] ],
    [ 0xA2, [ "STVA", "G", "1s", "!const1" ] ],
    [ 0xA3, [ "STVA", "G", "2u", "!const1" ] ],
    [ 0xA4, [ "STVA", "G", "2s", "!const1" ] ],
    [ 0xA5, [ "STVA", "G", "4", "!const1" ] ],
    [ 0xB0, [ "LDA", "!address" ] ],
    [ 0xB4, [ "PCVAL" ] ],
    [ 0xB8, [ "LVAR", "1u" ] ],
    [ 0xB9, [ "LVAR", "1s" ] ],
    [ 0xBA, [ "LVAR", "2u" ] ],
    [ 0xBB, [ "LVAR", "2s" ] ],
    [ 0xBC, [ "LVAR", "4" ] ],
    [ 0xBE, [ "ENAB" ] ],
    [ 0xC0, [ "MONITOR" ] ],
    [ 0xC4, [ "KILL" ] ],
    [ 0xD0, [ "LDB", "L" ] ],
    [ 0xD1, [ "LDB", "G" ] ],
    [ 0xD2, [ "STB", "L" ] ],
    [ 0xD3, [ "STB", "G" ] ],
    [ 0xD4, [ "LDV", "L", "2u", "LE" ] ],
    [ 0xD5, [ "LDV", "L", "2s", "LE" ] ],
    [ 0xD6, [ "LDV", "L", "4", "LE" ] ],
    [ 0xD7, [ "LDV", "G", "2u", "LE" ] ],
    [ 0xD8, [ "LDV", "G", "2s", "LE" ] ],
    [ 0xD9, [ "LDV", "G", "4", "LE" ] ],
    [ 0xDA, [ "STV", "L", "2u", "LE" ] ],
    [ 0xDB, [ "STV", "L", "2s", "LE" ] ],
    [ 0xDC, [ "STV", "L", "4", "LE" ] ],
    [ 0xDD, [ "STV", "G", "2u", "LE" ] ],
    [ 0xDE, [ "STV", "G", "2s", "LE" ] ],
    [ 0xDF, [ "STV", "G", "4", "LE" ] ],
  );
  return @instructions;
}

sub instruction_cfun {
    my ($op, $params) = @_;

    my $name = "instr";
    for (@{$params}) {
	next if /^!/;
	if (/^[0-9]/) {
	    $name .= lc($_);
	    next;
	}
	$name .= "_".lc($_);
    }
    return $name;
}

1;
