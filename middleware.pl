#!/usr/bin/perl

use strict;
use warnings;

use Attean;
use AtteanX::Parser::Turtle;
use Attean::TripleModel;
use AtteanX::Store::Memory;
use Attean::RDF;


# TODO some kind of socket that connects to the arduino and loops reading from there.

my $model = Attean::TripleModel->new(store => AtteanX::Store::Memory->new);
my $parser	= AtteanX::Parser::Turtle->new(handler => translate);
while () {
  $parser->parse_cb_from_io($fh, 'http://example.org/');
}


# Here, we give the each entry a timestamp, set their final URI and translate to SSN
sub translate {
  my $t = shift;
  my $subject 
