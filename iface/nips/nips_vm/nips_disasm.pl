#! /usr/bin/perl
# NIPS DisAsm - New Implementation of Promela Semantics (Dis-)Assembler
# Copyright (C) 2005: Stefan Schuermans <stefan@schuermans.info>
#                     Michael Weber <michaelw@i2.informatik.rwth-aachen.de>
#                     Lehrstuhl fuer Informatik II, RWTH Aachen
# Copyleft: GNU public license - http://www.gnu.org/copyleft/gpl.html

use strict;

# add directory of script to include directory
my $incdir=$0;
$incdir =~ s/[^\/]*$//;
$incdir = "./" if( $incdir eq "" );
push @INC, $incdir;

require "nips_asm_help.pl";
require "nips_asm_instr.pl";
my @instructions = get_instructions( );

print <<EOF;
NIPS DisAsm - New Implementation of Promela Semantics Dis-Assembler
Copyright (C) 2005: Stefan Schuermans <stefan\@schuermans.info>
                    Michael Weber <michaelw\@i2.informatik.rwth-aachen.de>
                    Lehrstuhl fuer Informatik II, RWTH Aachen
Copyleft: GNU public license - http://www.gnu.org/copyleft/gpl.html

EOF

# parse parameters

die "usage: perl nips_disasm.pl <input.b> [<output.d.s>]" if( @ARGV < 1 || @ARGV > 2 );
my ( $file_byte, $file_asm, $file_c ) = ( @ARGV[0], @ARGV[0], @ARGV[0] );
$file_asm =~ s/(\.b)?$/.d.s/;
$file_asm = @ARGV[1] if( @ARGV >= 2 );
$file_c =~ s/(\.b)?$/.c_stub/;

# read input

print "reading input file \"$file_byte\"...\n";

open BYTE, "<".$file_byte or die "could not open input file \"$file_byte\"";
binmode BYTE;

my $m;
read BYTE, $m, 8;
die "input file \"$file_byte\" is not a valid bytecode file" if( $m ne "NIPS v2 " );

my $sec_cnt = rd_word( \*BYTE );

my @modules = ( );

my $sec_no;
for( $sec_no = 0; $sec_no < $sec_cnt; $sec_no++ )
{
  # section header

  my $sec_type;
  read BYTE, $sec_type, 4;
  my $sec_sz = rd_dword( \*BYTE );
  my $sec_end = (tell BYTE) + $sec_sz;

  # module  

  if( $sec_type eq "mod " )
  {
    # module name

    my $mod_name = rd_string( \*BYTE );

    my $modflags = 0;
    my $bytecode = "";
    my $flags = [];
    my $strings = [];
    my $srclocs = [];
    my $strinfs = [];

    my $part_cnt = rd_word( \*BYTE );

    my $part_no;
    for( $part_no = 0; $part_no < $part_cnt; $part_no++ )
    {
      # part header

      my $part_type;
      read BYTE, $part_type, 4;
      my $part_sz = rd_dword( \*BYTE );
      my $part_end = (tell BYTE) + $part_sz;

      # module flags

      if( $part_type eq "modf" )
      {
        $modflags |= rd_dword( \*BYTE );
      }

      # bytecode

      elsif( $part_type eq "bc  " )
      {
        read BYTE, $bytecode, $part_sz;
      }

      # flag table

      elsif( $part_type eq "flag" )
      {
        my $flag_cnt = rd_word( \*BYTE );

        my $flag_no;
        $flags = [];
        for( $flag_no = 0; $flag_no < $flag_cnt; $flag_no++ )
        {
          # flag
          my $addr = rd_dword( \*BYTE );
          my $fl = rd_dword( \*BYTE );
          push @{$flags}, [ $addr, $fl ];
        }
      }

      # string table

      elsif( $part_type eq "str " )
      {
        my $str_cnt = rd_word( \*BYTE );

        my $str_no;
        $strings = [];
        for( $str_no = 0; $str_no < $str_cnt; $str_no++ )
        {
          push @{$strings}, (rd_string( \*BYTE ));
        }
      }

      # source location table

      elsif( $part_type eq "sloc" )
      {
        my $sloc_cnt = rd_word( \*BYTE );

        my $sloc_no;
        $srclocs = [];
        for( $sloc_no = 0; $sloc_no < $sloc_cnt; $sloc_no++ )
        {
          # source location
          my $addr = rd_dword( \*BYTE );
          my $line = rd_dword( \*BYTE );
          my $col = rd_dword( \*BYTE );
          push @{$srclocs}, [ $addr, $line, $col ];
        }
      }

      # structure information table

      elsif( $part_type eq "stin" )
      {
        my $stin_cnt = rd_word( \*BYTE );

        my $stin_no;
        $strinfs = [];
        for( $stin_no = 0; $stin_no < $stin_cnt; $stin_no++ )
        {
          # structure information
          my $addr = rd_dword( \*BYTE );
          my $code = rd_byte( \*BYTE );
          my $type = rd_string( \*BYTE );
          my $name = rd_string( \*BYTE );
          push @{$strinfs}, [ $addr, $code, $type, $name ];
        }
      }

      # unknown part

      else
      {
        seek BYTE, $part_sz, 1;
      }

      die "part $part_no of section $sec_no of input file \"$file_byte\" is corrupt" if( $part_end != (tell BYTE) );
    }

    push @modules, [ $mod_name, $modflags, $bytecode, $flags, $strings, $srclocs, $strinfs ];
  }

  # unknown section

  else
  {
    seek BYTE, $sec_sz, 1;
  }

  die "section $sec_no of input file \"$file_byte\" is corrupt" if( $sec_end != (tell BYTE) );
}

my $mod;
for $mod (@modules)
{
  my ($module, $modflags, $bytecode, $flags, $strings, $srclocs, $strinfs) = @{$mod};

  my ( $op );
  my $addr = 0;
  my @bytecodes = ( );
  my $pos = 0;
  my $len = length( $bytecode );
  while( $pos < $len )
  {
    
    $op = unpack( "C", substr( $bytecode, $pos, 1 ) );
    $pos += 1;

    # find instruction in table

    my $ok = 0;
    my ( $opcode, $params, $cflow );
    for (@instructions)
    {
      ( $opcode, $params, $cflow ) = @{$_};
      if( $opcode == $op )
      {
          $ok = 1;
          last;
      }
    }
    die "invalid opcode ".sprintf("0x%02X",$op)." at address ".sprintf("0x%08X",$addr) if( ! $ok );

    # get parameters

    my @bytecode = ( $opcode );
    my @parameters = ( );

    my $val;
    my @values;
    my $addr_tmp = $addr + 1;
    for (@{$params} )
    {
      # byte constant

      if( $_ eq "!const1" )
      {
        die "truncated instruction at address ".sprintf("0x%08X",$addr_tmp) if( $pos + 1 > $len );
        @values = unpack( "C", substr( $bytecode, $pos, 1 ) );
        $pos += 1;
        push @bytecode, @values;
        $val = @values[0];
        $val -= 256 if( $val >= 128 );
        push @parameters, $val;
        $addr_tmp += 1;
      }

      # word constant

      elsif( $_ eq "!const2" )
      {
        die "truncated instruction at address ".sprintf("0x%08X",$addr_tmp) if( $pos + 2 > $len );
        @values = unpack( "CC", substr( $bytecode, $pos, 2 ) );
        $pos += 2;
        push @bytecode, @values;
        $val = @values[0] << 8 | @values[1];
        $val -= 65536 if( $val >= 32768 );
        push @parameters, $val;
        $addr_tmp += 2;
      }

      # double-word constant

      elsif( $_ eq "!const4" )
      {
        die "truncated instruction at address ".sprintf("0x%08X",$addr_tmp) if( $pos + 4 > $len );
        @values = unpack( "CCCC", substr( $bytecode, $pos, 4 ) );
        $pos += 4;
        push @bytecode, @values;
        $val = @values[0] << 24 | @values[1] << 16 | @values[2] << 8 | @values[3];
        $val -= 4294967296 if( $val >= 2147483648 );
        push @parameters, $val;
        $addr_tmp += 4;
      }

      # register

      elsif( $_ eq "!reg" )
      {
        die "truncated instruction at address ".sprintf("0x%08X",$addr_tmp) if( $pos + 1 > $len );
        @values = unpack( "C", substr( $bytecode, $pos, 1 ) );
        $pos += 1;
        die "invalid register number ".@values[0]." at address ".sprintf("0x%08X",$addr_tmp) if( @values[0] >= 8 );
        push @bytecode, @values;
        push @parameters, "r".@values[0];
        $addr_tmp += 1;
      }

      # relative address given by label

      elsif( $_ eq "!addr" )
      {
        die "truncated instruction at address ".sprintf("0x%08X",$addr_tmp) if( $pos + 2 > $len );
        @values = unpack( "CC", substr( $bytecode, $pos, 2 ) );
        $pos += 2;
        push @bytecode, @values;
        $val = (@values[0] << 8 | @values[1]);
        $val -= 65536 if( $val >= 32768 );
        push @parameters, "addr:".$val;
        $addr_tmp += 2;
      }

      # absolute address given by label

      elsif( $_ eq "!address" )
      {
        die "truncated instruction at address ".sprintf("0x%08X",$addr_tmp) if( $pos + 4 > $len );
        @values = unpack( "CCCC", substr( $bytecode, $pos, 4 ) );
        $pos += 4;
        push @bytecode, @values;
        $val = (@values[0] << 24 | @values[1] << 16 | @values[2] << 8 | @values[3]);
        push @parameters, "address:".$val;
        $addr_tmp += 4;
      }

      # other parmeter type

      elsif( $_ =~ /^\!/ )
      {
        die "internal error: unknown parmeter type \"$_\"\n";
      }

      # fixed word

      else
      {
        push @parameters, $_;
      }
    }

    # save bytecode

    push @bytecodes, [ $addr, "", [ @bytecode ], [ @parameters ], [ @{$params} ], $cflow ];
    $addr = $addr_tmp;
  }

  $mod = [$module, $modflags, [@bytecodes], $flags, $strings, $srclocs, $strinfs];
}

# convert addresses into labels

print "converting addresses to labels...\n";

for $mod (@modules)
{
  my ($module, $modflags, $bytecodes, $flags, $strings, $srclocs, $strinfs) = @{$mod};

  for (@{$bytecodes})
  {
    my ( $addr, $label, $bc, $w, $params, $cflow ) = @{$_};
    for (@{$w})
    {

      # word is an address

      if( $_ =~ /^addr(ess)?:(-?[0-9]+)$/ )
      {
        my $rel = $1 eq "";
        my $ad = $2;
        $ad += $addr + @{$bc} if( $rel ); # convert to absolute address

        # find this address

        my $wo = "";
        for (@{$bytecodes})
        {
          my ( $addr, $label, $bc, $w, $params, $cflow ) = @{$_};
          if( $ad >= $addr && $ad < $addr + @{$bc} )
          {
            $label = "L_" . sprintf( "%08X", $addr );
            $_ = [ $addr, $label, $bc, $w, $params, $cflow ];
            $wo = $label;
            $wo .= "+" . ($ad - $addr) if( $ad != $addr );
            last;
          }
        }

        # update this word

        $wo = sprintf( "0x%08X", $ad ) if( $wo eq "" );
        $_ = $wo;
      }
    }
    # update this line
    $_ = [ $addr, @{$_}[1], $bc, $w, $params, $cflow ]; # keep label (might have been updated in inner loop
  }
}

# output disassembled code

print "writing output file \"$file_asm\"...\n";
open ASM, ">".$file_asm or die "could not open output file \"$file_asm\"";

print "writing output file \"$file_c\"...\n";
open CCODE, ">".$file_c or die "could not open output file \"$file_c\"";


print CCODE <<INITIAL_CCODE;
static inline void instr_exec( st_instr_context *p_ctx )
{
  t_pc pc_h = be2h_pc( p_ctx->p_proc->proc.pc );
#define INC_PC() p_ctx->p_proc->proc.pc = h2be_pc( pc_h = be2h_pc( p_ctx->p_proc->proc.pc ) + 1 )

  // select instruction (and advance program counter)
#ifdef DEBUG_INSTR
  printf( "DEBUG (before instr): " ); global_state_print( p_ctx->p_glob );
#endif
  switch(pc_h) {
INITIAL_CCODE

for $mod (@modules)
{
  my ($module, $modflags, $bytecodes, $flags, $strings, $srclocs, $strinfs) = @{$mod};

  # module name
  print ASM "!module \"" . escape_str( $module ) . "\"\n\n";

  # module flags

  print ASM "!modflags monitor\n\n" if( $modflags & 0x00000001 );

  # byte code

  my $old_cflow = 1;
  for (@{$bytecodes})
  {
    my ( $addr, $label, $bc, $w, $params, $cflow ) = @{$_};

    # source code

    if( $label ne "" )
    {
      print ASM "$label:";
      my $i;
      for( $i = 0; $i < 11 - length( $label ) - 1; $i++ ) { print ASM " "; }
    }
    else
    {
      print ASM "           ";
    }
    print ASM " $_" for (@{$w});
    print ASM "\n";

    printf CCODE "  case 0x%x:\n", $addr if $old_cflow or @{$_}[1] ne "";
    print  CCODE "\tINC_PC();\t\t".instruction_cfun(0, $params)."(p_ctx);\n";
    print  CCODE "\tbreak;\n" if $cflow;
    $old_cflow = $cflow;
  }
  print ASM "\n";

  # flag table  

  my $i;
  for( $i = 0; $i < @{$flags}; $i++ )
  {
    my ($addr, $fl) = @{@{$flags}[$i]};

    printf ASM "!flags_addr 0x%08X", $addr;
    print ASM " progress" if( $fl & 0x00000001 );
    print ASM " accept" if( $fl & 0x00000002 );
    print ASM "\n";
  }
  print ASM "\n";

  # string table

  for( $i = 0; $i < @{$strings}; $i++ )
  {
    my $str = @{$strings}[$i];
    print ASM "!string $i \"" . escape_str( $str ) . "\"\n" if( $str ne undef );
  }
  print ASM "\n";

  # source location table

  for( $i = 0; $i < @{$srclocs}; $i++ )
  {
    my ($addr, $line, $col) = @{@{$srclocs}[$i]};
    printf ASM "!srcloc_addr 0x%08X %d %d\n", $addr, $line, $col;
  }
  print ASM "\n";

  # structure information table

  for( $i = 0; $i < @{$strinfs}; $i++ )
  {
    my ($addr, $code, $type, $name) = @{@{$strinfs}[$i]};
    printf ASM "!strinf_addr 0x%08X", $addr;
    if( $code == 0x00 )
      { print ASM " begin "; }
    elsif( $code == 0x01 )
      { print ASM " end "; }
    elsif( $code == 0x02 )
      { print ASM " middle "; }
    else
      { print ASM " unknown "; }
    print ASM $type;
    print ASM " $name" if( $name ne "" );
    print ASM "\n";
  }
  print ASM "\n";
}

print CCODE <<FINAL_CCODE;
  default:
	instr_err( p_ctx, IE_BYTECODE );
  }
}
FINAL_CCODE

print "done...\n\n";

