#! /usr/bin/perl
# NIPS Asm - New Implementation of Promela Semantics Assembler
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

my $libprefix = $ENV{'NIPS_HOME'} || ".";

require $libprefix."/nips_asm_help.pl";
require $libprefix."/nips_asm_instr.pl";
my @instructions = get_instructions( );

print <<EOF;

NIPS Asm - New Implementation of Promela Semantics Assembler
Copyright (C) 2005: Stefan Schuermans <stefan\@schuermans.info>
                    Michael Weber <michaelw\@i2.informatik.rwth-aachen.de>
                    Lehrstuhl fuer Informatik II, RWTH Aachen
Copyleft: GNU public license - http://www.gnu.org/copyleft/gpl.html

EOF

# parse parameters

die "usage: perl nips_asm.pl <input.s> [<output.b> [<output.l>]]" if( @ARGV < 1 || @ARGV > 3 );
my ( $file_asm, $file_byte, $file_list ) = ( @ARGV[0], @ARGV[0], @ARGV[0] );
$file_byte =~ s/(\.s)?$/.b/;
$file_byte = @ARGV[1] if( @ARGV >= 2 );
$file_list =~ s/(\.s)?$/.l/;
$file_list = @ARGV[2] if( @ARGV >= 3 );

# parse input

print "parsing input file \"$file_asm\"...\n";

open ASM, "<".$file_asm or die "could not open input file \"$file_asm\"";

my ( $line_no, $line, $module );
my $modflags = 0;
my $addr = 0;
my @bytecodes = ( );
my @flags = ( );
my @strings = ( );
my @srclocs = ( );
my @strinfs = ( );
$module = "";
my @modules = ( );
for( $line_no = 1; $line = <ASM>; $line_no++ )
{
  
  # remove newline, whitespace, comment

  chomp $line;
  chomp $line;
  $line =~ s/\t/ /g;
  $line =~ s/^ *([^;]*)(;.*)?$/\1/;

  # get label and split words

  $line =~ /^(([A-Za-z][A-Za-z0-9_]*): *)?(.*)$/;
  my $label = $2;
  my @words = split /[ ,]+/, $3;

  # get string possibly contained in line

  my $str = undef;
  $str = eval( $1 ) if( $line =~ /^[^"]*("([^"]|\\.)*").*$/ );

  # empty line

  if( @words <= 0 || (@words == 1 && @words[0] eq "") )
  {
    # save empty bytecode for this line if there is a label
    push @bytecodes, [ $addr, $label, [ ], [ ] ] if( $label ne "" );
    next;
  }

  # start of new module

  if( @words[0] eq "!module" )
  {
    die "\"!module\" needs a string in line $line_no" if( $str eq undef );
    push @modules, [ $module, $modflags, [ @bytecodes ], [ @strings ] ] if( @bytecodes > 0 or @strings > 0 );
    $module = $str;
    $modflags = 0;
    $addr = 0;
    @bytecodes = ( );
    @flags = ( );
    @strings = ( );
    @srclocs = ( );
    @strinfs = ( );
    next;
  }

  # module flags

  if( @words[0] eq "!modflags" )
  {
    my $flag;
    foreach $flag (@words)
    {
      if( $flag eq "monitor" ) { $modflags |= 0x00000001; }
    }
    next;
  }

  # flags for address

  if( @words[0] eq "!flags" || @words[0] eq "!flags_addr" )
  {
    my $ad = $addr;
    my $i = 1;
    if( @words[0] eq "!flags_addr" )
    {
      if( @words[1] =~ /^0*[xX]/ )
        { $ad = hex( @words[1] ); }
      else
        { $ad = int( @words[1] ); }
      $i = 2;
    }
    my $fl = 0;
    for( ; $i < @words; $i++ )
    {
      if( @words[$i] eq "progress" )
        { $fl |= 0x00000001; }
      elsif( @words[$i] eq "accept" )
        { $fl |= 0x00000002; }
      else
        { die "unknown flag \"@words[$i]\" in line $line_no"; }
    }
    if( @flags > 0 && @{@flags[@flags-1]}[0] > $ad ) {
	die "flags out of order in line $line_no"
    } elsif (@flags > 0 && @{@flags[@flags-1]}[0] == $ad) {
	# add more flags
	@{@flags[@flags-1]}[1] |= $fl;
    } else {
        push @flags, [ $ad, $fl ] if( $fl != 0 );
    }	
    next;
  }

  # string to put into string table

  if( @words[0] eq "!string" )
  {
    die "\"!string\" needs a number in line $line_no" if( @words[1] !~ /^[0-9]+$/ );
    die "\"!string\" needs a string in line $line_no" if( $str eq undef );
    my $i = int( @words[1] );
    die "duplicate definition of string $i in line $line_no" if( $i < @strings and @strings[$i] ne undef );
    my $j;
    for( $j = @strings; $j < $i; $j++ )
      { @strings[$j] = undef; }
    @strings[$i] = $str;
    next;
  }  

  # source location

  if( @words[0] eq "!srcloc" || @words[0] eq "!srcloc_addr" )
  {
    my $ad = $addr;
    my $i = 1;
    if( @words[0] eq "!srcloc_addr" )
    {
      if( @words[1] =~ /^0*[xX]/ )
        { $ad = hex( @words[1] ); }
      else
        { $ad = int( @words[1] ); }
      $i = 2;
    }
    my $line = int( @words[$i] );
    my $col = int( @words[$i+1] );
    die "source location out of order in line $line_no" if( @srclocs > 0 && @{@srclocs[@srclocs-1]}[0] > $ad );
    push @srclocs, [ $ad, $line, $col ];
    next;
  }

  # structure information

  if( @words[0] eq "!strinf" || @words[0] eq "!strinf_addr" )
  {
    my $ad = $addr;
    my $i = 1;
    if( @words[0] eq "!strinf_addr" )
    {
      if( @words[1] =~ /^0*[xX]/ )
        { $ad = hex( @words[1] ); }
      else
        { $ad = int( @words[1] ); }
      $i = 2;
    }
    my $code = 0xFF;
    $code = 0x00 if( @words[$i] eq "begin" );
    $code = 0x01 if( @words[$i] eq "end" );
    $code = 0x02 if( @words[$i] eq "middle" );
    my $type = @words[$i+1] . "";
    die "invalid type \"$type\" in structure information in line $line_no" if( $type !~ /^[A-Za-z0-9_]+$/ );
    my $name = @words[$i+2] . "";
    die "invalid name \"$name\" in structure information in line $line_no" if( $name !~ /^[A-Za-z0-9_.]*$/ );
    die "structure information out of order in line $line_no" if( @strinfs > 0 && @{@strinfs[@strinfs-1]}[0] > $ad );
    push @strinfs, [ $ad, $code, $type, $name ];
    next;
  }

  # find instruction in table

  my $ok = 0;
  my ( $opcode, $params );
  for (@instructions)
  {
    ( $opcode, $params ) = @{$_};
    if( @words == @{$params} )
    {
      my $i;
      for( $i = 0; $i < @{$params}; $i++ )
      {
        my $param = @{$params}[$i];
        my $word = @words[$i];
        last if( $param !~ /^\!/ and $param ne $word );
      }
      if( $i >= @{$params} )
      {
        $ok = 1;
        last;
      }
    }
  }
  die "invalid instruction \"@words\" in line $line_no" if( ! $ok );

  # process parameters and generate bytecode

  my @bytecode = ( $opcode );
  my $i;
  for( $i = 0; $i < @{$params}; $i++ )
  {

    # byte constant

    if( @{$params}[$i] eq "!const1" )
    {
      die "invalid constant \"".@words[$i]."\" in line $line_no" if( @words[$i] !~ /^-?[0-9]+$/ );
      my $val = int( @words[$i] );
      die "1-byte constant \"".@words[$i]."\" in line $line_no is out of range" if( $val > 0xFF or $val < -0x80 );
      push @bytecode, $val & 0xFF;
    }

    # word constant

    elsif( @{$params}[$i] eq "!const2" )
    {
      die "invalid constant \"".@words[$i]."\" in line $line_no" if( @words[$i] !~ /^-?[0-9]+$/ );
      my $val = int( @words[$i] );
      die "2-byte constant \"".@words[$i]."\" in line $line_no is out of range" if( $val > 0xFFFF or $val < -0x8000 );
      push @bytecode, ($val >> 8) & 0xFF;
      push @bytecode, $val & 0xFF;
    }

    # double-word constant

    elsif( @{$params}[$i] eq "!const4" )
    {
      die "invalid constant \"".@words[$i]."\" in line $line_no" if( @words[$i] !~ /^-?[0-9]+$/ );
      my $val = int( @words[$i] );
      push @bytecode, ($val >> 24) & 0xFF;
      push @bytecode, ($val >> 16) & 0xFF;
      push @bytecode, ($val >> 8) & 0xFF;
      push @bytecode, $val & 0xFF;
    }

    # register

    elsif( @{$params}[$i] eq "!reg" )
    {
      die "invalid register \"".@words[$i]."\" in line $line_no" if( @words[$i] !~ /^r([0-7])$/ );
      push @bytecode, int( $1 );
    }

    # relative address given by label

    elsif( @{$params}[$i] eq "!addr" )
    {
      die "invalid label \"".@words[$i]."\" in line $line_no" if( @words[$i] !~ /^[A-Za-z][A-Za-z0-9_]*$/ );
      push @bytecode, "addr1:".@words[$i]; # relative address of label takes 2 bytes
      push @bytecode, "addr0:".@words[$i];
    }

    # absolute address given by label

    elsif( @{$params}[$i] eq "!address" )
    {
      die "invalid label \"".@words[$i]."\" in line $line_no" if( @words[$i] !~ /^[A-Za-z][A-Za-z0-9_]*$/ );
      push @bytecode, "address3:".@words[$i]; # absolute address of label takes 4 bytes
      push @bytecode, "address2:".@words[$i];
      push @bytecode, "address1:".@words[$i];
      push @bytecode, "address0:".@words[$i];
    }

    # other parmeter type

    elsif( @{$params}[$i] =~ /^\!/ )
    {
      die "internal error: unknown parmeter type \"".@{$params}[$i]."\"\n";
    }
  }

  # save bytecode of this line

  push @bytecodes, [ $addr, $label, [ @bytecode ], [ @words ] ];
  $addr += @bytecode
}
push @modules, [ $module, $modflags, [ @bytecodes ], [ @flags ], [ @strings ], [ @srclocs], [ @strinfs ] ] if( @bytecodes > 0 or @strings > 0 );
die "no code found" if( @modules <= 0 );

# convert labels into addresses

print "converting labels to addresses...\n";

for (@modules)
{
  my ($module, $modflags, $bytecodes, $flags, $strings, $srclocs, $strinfs) = @{$_};

  for (@{$bytecodes})
  {
    my ( $addr, $label, $bc, $w ) = @{$_};
    for (@{$bc})
    {

      # byte in bytecode is a label

      if( $_ =~ /^addr(ess)?([0123]):([A-Za-z][A-Za-z0-9_]*)$/ )
      {
        my $rel = $1 eq "";
        my $byte = $2;
        my $lab = $3;

        # find declaration of this label

        my $ok = 0;
        my $ad = "";
        for (@{$bytecodes})
        {
          my ( $addr, $label, $bc, $w ) = @{$_};
          if( $label eq $lab )
          {
            $ad = $addr;
            $ok = 1;
            last;
          }
        }
        die "label \"$lab\" is not declared in module \"$module\"" if( ! $ok );

        # convert address into relative one

        if( $rel )
        {
          $ad -= $addr + @{$bc};
          die "destination label \"".$lab."\" in module \"$module\" is out of range" if( $ad > 0x7FFF or $ad < -0x8000 );
        }

        # put right byte address into bytecode

        $_ = ($ad >> ($byte * 8)) & 0xFF;
      }
    }

    # update this line

    $_ = [ $addr, $label, $bc, $w ];
  }
}

# output bytecode and listing

print "writing output files \"$file_byte\" and \"$file_list\"...\n";

open BYTE, ">".$file_byte or die "could not open bytecode output file \"$file_byte\"";
binmode BYTE;
open LIST, ">".$file_list or die "could not open list output file \"$file_list\"";

# file header

print BYTE "NIPS v2 ";
wr_word( \*BYTE, @modules + 0 );
print LIST "# this code was assembled according to \"NIPS v2 \"\n\n";

# modules

for (@modules)
{
  my ($module, $modflags, $bytecodes, $flags, $strings, $srclocs, $strinfs) = @{$_};

  # module header

  # sec_type
  print BYTE "mod ";
  # sec_sz (0 for now)
  my $sec_sz_pos = wr_size_tmp( \*BYTE );

  print LIST "!module \"" . escape_str( $module ) . "\"\n\n";

  # module_name
  wr_string( \*BYTE, $module );

  # part_cnt
  wr_word( \*BYTE, 6 );

  # module flags

  print BYTE "modf";
  wr_dword( \*BYTE, 4 );
  wr_dword( \*BYTE, $modflags );

  print LIST "!modflags";
  print LIST " monitor" if( $modflags & 0x00000001 );
  print LIST "\n\n";

  # code

  # part_type
  print BYTE "bc  ";
  # part_sz (0 for now)
  my $part_sz_pos = wr_size_tmp( \*BYTE );

  for (@{$bytecodes})
  {
    my ( $addr, $label, $bc, $w ) = @{$_};

    # byte code

    wr_byte( \*BYTE, $_ ) for (@{$bc});

    # hex dump of bytecode

    printf LIST "0x%08X:", $addr;
    printf LIST " 0x%02X", $_ for (@{$bc});

    # indentation

    my $i;
    for( $i = 0; $i < 48 - 7 - 5 * @{$bc}; $i++ ) { print LIST " "; }

    # source code

    print LIST " #";
    print LIST " $label:" if( $label ne "" );
    print LIST " $_" for (@{$w});
    print LIST "\n";
  }
  print LIST "\n";

  # part_sz
  wr_size_fillin( \*BYTE, $part_sz_pos );

  # flag table

  # part_type
  print BYTE "flag";
  # part_sz
  $part_sz_pos = wr_size_tmp( \*BYTE );

  # flag_cnt
  wr_word( \*BYTE, @{$flags} + 0 );

  my $i;
  for( $i = 0; $i < @{$flags}; $i++ )
  {
    my ($addr, $fl) = @{@{$flags}[$i]};
    # flag
    wr_dword( \*BYTE, $addr );
    wr_dword( \*BYTE, $fl );

    printf LIST "!flags_addr 0x%08X", $addr;
    print LIST " progress" if( $fl & 0x00000001 );
    print LIST " accept" if( $fl & 0x00000002 );
    print LIST "\n";
  }
  print LIST "\n";

  # part_sz
  wr_size_fillin( \*BYTE, $part_sz_pos );

  # string table

  # part_type
  print BYTE "str ";
  # part_sz
  $part_sz_pos = wr_size_tmp( \*BYTE );

  # str_cnt
  wr_word( \*BYTE, @{$strings} + 0 );

  for( $i = 0; $i < @{$strings}; $i++ )
  {
    my $str = @{$strings}[$i];
    if( $str ne undef )
    {
      wr_string( \*BYTE, $str );
      print LIST "!string $i \"" . escape_str( $str ) . "\"\n";
    }
    else
    {
      # empty string
      wr_string( \*BYTE, "" );
    }
  }
  print LIST "\n";

  # part_sz
  wr_size_fillin( \*BYTE, $part_sz_pos );

  # source location table

  # part_type
  print BYTE "sloc";
  # part_sz
  $part_sz_pos = wr_size_tmp( \*BYTE );

  # sloc_cnt
  wr_word( \*BYTE, @{$srclocs} + 0 );

  for( $i = 0; $i < @{$srclocs}; $i++ )
  {
    my ($addr, $line, $col) = @{@{$srclocs}[$i]};
    # srcloc
    wr_dword( \*BYTE, $addr );
    wr_dword( \*BYTE, $line );
    wr_dword( \*BYTE, $col );

    printf LIST "!srcloc_addr 0x%08X %d %d\n", $addr, $line, $col;
  }
  print LIST "\n";

  # part_sz
  wr_size_fillin( \*BYTE, $part_sz_pos );

  # structure information table

  # part_type
  print BYTE "stin";
  # part_sz
  $part_sz_pos = wr_size_tmp( \*BYTE );

  # stin_cnt
  wr_word( \*BYTE, @{$strinfs} + 0 );

  for( $i = 0; $i < @{$strinfs}; $i++ )
  {
    my ($addr, $code, $type, $name) = @{@{$strinfs}[$i]};
    # strinf
    wr_dword( \*BYTE, $addr );
    wr_byte( \*BYTE, $code );
    wr_string( \*BYTE, $type );
    wr_string( \*BYTE, $name );

    printf LIST "!strinf_addr 0x%08X", $addr;
    if( $code == 0x00 )
      { print LIST " begin "; }
    elsif( $code == 0x01 )
      { print LIST " end "; }
    elsif( $code == 0x02 )
      { print LIST " middle "; }
    else
      { print LIST " unknown "; }
    print LIST $type;
    print LIST " $name" if( $name ne "" );
    print LIST "\n";
  }
  print LIST "\n";

  # part_sz
  wr_size_fillin( \*BYTE, $part_sz_pos );

  # end of section

  # sec_sz
  wr_size_fillin( \*BYTE, $sec_sz_pos );
}
print "done...\n\n";

