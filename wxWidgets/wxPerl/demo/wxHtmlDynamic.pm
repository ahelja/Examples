#############################################################################
## Name:        demo/wxHtmlDynamic.pm
## Purpose:     Dynamically generated HTML ( via Wx::FsHandler )
## Author:      Mattia Barbon
## Modified by:
## Created:     18/04/2002
## RCS-ID:      $Id: wxHtmlDynamic.pm,v 1.4 2004/10/19 20:28:06 mbarbon Exp $
## Copyright:   (c) 2002 Mattia Barbon
## Licence:     This program is free software; you can redistribute it and/or
##              modify it under the same terms as Perl itself
#############################################################################

package HtmlDynamicDemo;

use strict;

use Wx qw(:sizer);
use Wx::Html;
use Wx::FS;

sub window {
  shift;
  my $parent = shift;

  Wx::FileSystem::AddHandler( new MyFSHandler() );

  my $sizer = Wx::BoxSizer->new( wxVERTICAL );
  my $panel = Wx::Panel->new( $parent, -1 );
  my $htmlwin = Wx::HtmlWindow->new( $panel, -1 );

  $sizer->Add( $htmlwin, 1, wxGROW );
  $panel->SetSizer( $sizer );
  $htmlwin->LoadPage( "my://foo.bar/baz" );

  return $panel;
}

sub description {
  return <<EOT;
<html>
<head>
  <title>Dynamic HTML generation</title>
</head>
<body>
<h3>Dynamic HTML generation</h3>

<p>
  This (silly) example shows how to subclass Wx::FileSystemHandler
  in order to provide dynamic HTML generation.
</p>

</body>
</html>
EOT
}

package MyFSHandler;

use base qw(Wx::PlFileSystemHandler);

sub new {
  my $class = shift;
  my $this = $class->SUPER::new( @_ );

  return $this;
}

sub CanOpen {
  my $file = $_[1];

  return scalar( $file =~ m{^my://} );
}

# no findfirst/findnext, not needed for this example

my @f;

sub OpenFile {
  my( $this, $fs, $location ) = @_;
  my $loc = $location;

  $loc =~ s{^my://}{};

  my $text = join '',
    map { qq{<a href="my://$_">}.( $loc ne $_ ? $_ : 'Here' ).qq{</a><br>} }
    ( 'foo.bar/baz', 'Here, there, everywhere',
      'Somewhere else', 'A galaxy far, far away' );

  my $string = <<EOT;
<html>
<head>
  <title>$loc</title>
</head>
<body>
<h1>$loc</h1>

Something useful here<br><br>

Links:<br>
$text

</body>
</html>
EOT

  my $f= Wx::PlFSFile->new( IOScalar->new( $string ), $location, 'text/html',
                            '' );
  return $f;
}

# a small class to di I/O ( ok, just I/... )
# from a scalar, for serious use, look at something like IO::Scalar or
# ( if you use Perl 5.8 or better ) open FOO, \$scalar
package IOScalar;

use Symbol qw(gensym);

sub new {
  shift;
  my $this = gensym();

  tie *$this, 'IOScalar::Tie', shift;

  return $this;
}

package IOScalar::Tie;

sub TIEHANDLE {
  my $class = shift;
  my $string = shift;

  my $this = bless { STRING => "$string", OFFSET => 0 }, $class;

  return $this;
}

sub READ {
  my $this = $_[0];
  return 0 if $this->{OFFSET} >= length( $this->{STRING} );
  my $remlen = length( $this->{STRING} ) - $this->{OFFSET};

  my $len = $_[2];
  my $offset = $_[3] || 0;

  $len = $len > $remlen ? $remlen : $len;
  if( $offset ) {
    die "I'm too lazy: use IO::Scalar!";
  } else {
    $_[1] = substr( $this->{STRING}, $this->{OFFSET}, $len );
  }
  $this->{OFFSET} += $len;
  return $len;
}

1;

# local variables:
# mode: cperl
# end:
