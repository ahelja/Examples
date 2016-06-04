#############################################################################
## Name:        demo/wxGridSizer.pm
## Purpose:     wxPerl demo helper for Wx::GridSizer
## Author:      Mattia Barbon
## Modified by:
## Created:     03/07/2002
## RCS-ID:      $Id: wxGridSizer.pm,v 1.3 2004/10/19 20:28:06 mbarbon Exp $
## Copyright:   (c) 2002 Mattia Barbon
## Licence:     This program is free software; you can redistribute it and/or
##              modify it under the same terms as Perl itself
#############################################################################

package GridSizerDemo;

use strict;

sub window {
  shift;
  my $parent = shift;

  my $dialog = GSDemoWindow->new( $parent );
  $dialog->ShowModal;
  $dialog->Destroy;

  return undef;
}

sub description {
  return <<EOT;
<html>
<head>
  <title>Wx::GridSizer
</head>
<body>
<h3>Wx::GridSizer</h3>

<p>
  This sizer lays out windows in a grid having all rows and columns
  of the same size; if you want more flexibility, look at Wx::FlexGridSizer.
</p>

</body>
</html>
EOT
}

package GSDemoWindow;

use strict;
use base qw(Wx::Dialog);
use Wx qw(:sizer wxDefaultPosition wxDefaultSize
          wxDEFAULT_DIALOG_STYLE wxRESIZE_BORDER wxID_OK);

sub new {
  my $class = shift;
  my $this = $class->SUPER::new( undef, -1, "Wx::GridSizer",
                                 wxDefaultPosition, wxDefaultSize,
                                 wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );

  # top level sizer
  my $tsz = Wx::GridSizer->new( 5, 5, 1, 1, );

  for my $i ( 1 .. 25 ) {
    my $grow = ( $i % 2 ) * wxGROW;

    $tsz->Add( Wx::Button->new( $this, wxID_OK, "Button $i" ),
               0, $grow|wxALL, 2 );
  }

  # tell we want automatic layout
  $this->SetAutoLayout( 1 );
  $this->SetSizer( $tsz );
  # size the window optimally and set its minimal size
  $tsz->Fit( $this );
  $tsz->SetSizeHints( $this );

  return $this;
}

1;

# local variables:
# mode: cperl
# end:
