#!/usr/bin/perl -w

#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

sub read_file {
    my ($path) = @_;
    my ($cur_container, $id, $tag, $sym, $n);

    $cur_container = 'none';
    open(P, "readelf --debug-dump=info $path |") || die("readelf: $!\n");
    while (<P>) {
        chop;
        if (/Abbrev Number:/) {
            ($id, $tag) = $_ =~ /<([[:xdigit:]]+)>:\s+Abbrev.*\(DW_TAG_([[:word:]]+)\)$/;
            if (defined($tag)) {
                $symtab{$id}{'id'} = $id;
                $sym = $symtab{$id};
                $sym->{'tag'} = $tag;
                if ($tag eq 'structure_type' ||
                    $tag eq 'union_type') {
                    $cur_container = $id;
                    $containers{$cur_container} = '';
                } elsif ($tag eq 'member') {
                    $containers{$cur_container} .= " $id";
                }
            } elsif (/Abbrev Number: 0/) {
                undef($sym);
                $cur_container = 'none';
            } else {
                die("can't parse $_\n");
            }
        } elsif (/DW_AT_name/) {
           ($n) = $_ =~ /([[:word:]]+)$/;
           die("can't parse $_\n") if !defined($n);
           $sym->{'name'} = $n if defined($sym);
        } elsif (/DW_AT_type/) {
           ($n) = $_ =~ /DW_AT_type\s+:\s+<0x([[:xdigit:]]+)>$/;
           die("can't parse $_\n") if !defined($n);
           $sym->{'type'} = $n if defined($sym);
        } elsif (/DW_AT_sibling/) {
           ($n) = $_ =~ /DW_AT_sibling\s+:\s+<0x([[:xdigit:]]+)>$/;
           die("can't parse $_\n") if !defined($n);
           $sym->{'sibling'} = $n if defined($sym);
        } elsif (/DW_AT_data_member_location/) {
           ($n) = $_ =~ /DW_AT_data_member_location:\s+([[:digit:]]+)$/;
           die("can't parse $_\n") if !defined($n);
           $sym->{'offset'} = $n if defined($sym);
        }
    }
    close(P);
}

sub declare_union {
    my ($container, $pfx, $boffs) = @_;
    my ($sym, $id, $offs, $name, $npfx);

    $pfx = $pfx . $container->{'name'} . "_" if defined($container->{'name'});
    $_ = $containers{$container->{'id'}};
    foreach $id (split()) {
        $sym = $symtab{$id};
        $offs = 0;
        $name = $sym->{'name'};
        if (defined($name)) {
            printf "\t.equ\t%s%s, %s\n", $pfx, $name, $boffs + $offs;
        }
        $container = $symtab{$sym->{'type'}};
        if (defined($name)) {
            $npfx = $pfx . $name . "_";
        } else {
            $npfx = $pfx;
        }
        if ($container->{'tag'} eq 'structure_type') {
            &declare_struct($container, $npfx, $boffs + $offs)
        } elsif ($container->{'tag'} eq 'union_type') {
            &declare_union($container, $npfx, $boffs + $offs)
        }
    }
}

sub declare_struct {
    my ($container, $pfx, $boffs) = @_;
    my ($sym, $id, $offs, $npfx);

    $pfx = $pfx . $container->{'name'} . "_" if defined($container->{'name'});
    $pfx =~ s/_s_$/_/;

    $_ = $containers{$container->{'id'}};
    foreach $id (split()) {
        $sym = $symtab{$id};
        $offs = $sym->{'offset'};
        next if !defined($offs);
        $name = $sym->{'name'};
        if (defined($name)) {
            printf "\t.equ\t%s%s, %s\n", $pfx, $name, $boffs + $offs;
        }
        $container = $symtab{$sym->{'type'}};
        if (defined($name)) {
            $npfx = $pfx . $name . "_";
        } else {
            $npfx = $pfx;
        }
        if ($container->{'tag'} eq 'structure_type') {
            &declare_struct($container, $npfx, $boffs + $offs)
        } elsif ($container->{'tag'} eq 'union_type') {
            &declare_union($container, $npfx, $boffs + $offs)
        }
    }
}

sub dump_tree {
    foreach $c (sort(keys(%containers))) {
        $container = $symtab{$c};
        $indent = 0;
        if ($container->{'tag'} eq 'structure_type') {
            printf "struct";
        } else {
            printf "union";
        }
        $n = $container->{'name'};
        if (defined($n)) {
            printf " $n";
        } else {
            printf " <$c>";
        }
        printf " {\n";
        $indent += 4;
        $_ = $containers{$c};
        foreach $id (split()) {
            $sym = $symtab{$id};
            printf "%*s", $indent, "";
            printf "[+$sym->{'offset'}]" if defined($sym->{'offset'});
            printf " $sym->{'name'}" if defined($sym->{'name'});
            if (defined($sym->{'type'})) {
                $id = $sym->{'type'};
                printf " : ";
                while (defined($id)) {
                    $sym = $symtab{$id};
                    last if !defined($sym);
                    if (defined($sym->{'name'})) {
                        printf "->$sym->{'name'}";
                    } else {
                        $tag = $sym->{'tag'};
                        if ($tag eq 'structure_type' || $tag eq 'union_type') {
                            if ($tag eq 'structure_type') {
                                printf "->struct";
                            } else {
                                printf "->union";
                            }
                            if (defined($sym->{'name'})) {
                                printf " $sym->{'name'}";
                            } else {
                                printf " <$id>";
                            }
                        } else {
                            printf "->$sym->{'tag'}";
                        }
                    }
                    $id = $sym->{'type'};
                }
                printf "\n";
            }
        }
        $indent -= 4;
        printf "}\n";
    }
}

$debug = 0;
foreach $a (@ARGV) {
    if ($a eq '--debug') {
        $debug = 1;
    } else {
        $path = $a;
    }
}
die("usage $0 path\n") if !defined($path);

&read_file($path);

foreach $c (sort(keys(%containers))) {
    $container = $symtab{$c};
    if ($container->{'tag'} eq 'structure_type' && defined($container->{'name'})) {
        printf "\t# struct %s\n", $container->{'name'};
        &declare_struct($container, '', 0);
    }
}

&dump_tree() if $debug;
