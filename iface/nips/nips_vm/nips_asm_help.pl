# NIPS Asm - New Implementation of Promela Semantics Assembler
# Copyright (C) 2005: Stefan Schuermans <stefan@schuermans.info>
#                     Michael Weber <michaelw@i2.informatik.rwth-aachen.de>
#                     Lehrstuhl fuer Informatik II, RWTH Aachen
# Copyleft: GNU public license - http://www.gnu.org/copyleft/gpl.html

use strict;

# escape a string
sub escape_str
{
  my $str = shift;
  $str =~ s/\0/\\0/g;
  $str =~ s/\r/\\r/g;
  $str =~ s/\n/\\n/g;
  $str =~ s/\t/\\t/g;
  $str =~ s/"/\\"/g;
  $str =~ s/'/\\'/g;
  my $i;
  for( $i = 1; $i < 32; $i++ )
  {
    my $c = pack( "C", $i );
    my $h = sprintf( "%02X", $i );
    $str =~ s/$c/\\x$h/g;
  }
  return $str;
}

# convert a byte, a word, a dword to binary
sub byte2bin
{
  my $value = shift;
  return pack( "C", $value & 0xFF );
}
sub word2bin
{
  my $value = shift;
  return pack( "CC", ($value >> 8) & 0xFF,
                     $value & 0xFF );
}
sub dword2bin
{
  my $value = shift;
  return pack( "CCCC", ($value >> 24) & 0xFF,
                       ($value >> 16) & 0xFF,
                       ($value >> 8) & 0xFF,
                       $value & 0xFF );
}

# convert a byte, a word, a dword from binary
sub bin2byte
{
  my @data = unpack( "C", shift );
  return @data[0];
}
sub bin2word
{
  my @data = unpack( "CC", shift );
  return @data[0] << 8 | @data[1];
}
sub bin2dword
{
  my @data = unpack( "CCCC", shift );
  return @data[0] << 24 | @data[1] << 16 | @data[2] << 8 | @data[3];
}

# write a byte, a word, a dword to a binary file
sub wr_byte
{
  my $filehandle = shift;
  my $byte = shift;
  print $filehandle byte2bin( $byte );
}
sub wr_word
{
  my $filehandle = shift;
  my $word = shift;
  print $filehandle word2bin( $word );
}
sub wr_dword
{
  my $filehandle = shift;
  my $dword = shift;
  print $filehandle dword2bin( $dword );
}

# read a byte, a word, a dword from a binary file
sub rd_byte
{
  my $filehandle = shift;
  my $data;
  read $filehandle, $data, 1;
  return bin2byte( $data );
}
sub rd_word
{
  my $filehandle = shift;
  my $data;
  read $filehandle, $data, 2;
  return bin2word( $data );
}
sub rd_dword
{
  my $filehandle = shift;
  my $data;
  read $filehandle, $data, 4;
  return bin2dword( $data );
}

# write a string to a binary file
sub wr_string
{
  my $filehandle = shift;
  my $str = shift;
  wr_word( $filehandle, length( $str ) + 1 );
  print $filehandle $str . "\0";
}

# read a string from a binary file
sub rd_string
{
  my $filehandle = shift;
  my $str_sz = rd_word( $filehandle );
  my $str;
  read $filehandle, $str, $str_sz;
  $str =~ s/\0.*$//;
  return $str;
}

# write size to binary file
sub wr_size_tmp
{
  my $filehandle = shift;
  my $pos = tell $filehandle;
  wr_dword( $filehandle, 0 );
  return $pos;
}
sub wr_size_fillin
{
  my $filehandle = shift;
  my $sz_pos = shift;
  my $pos = tell $filehandle;
  seek $filehandle, $sz_pos, 0;
  wr_dword( $filehandle, $pos - $sz_pos - 4 );
  seek $filehandle, $pos, 0;
}

1;
