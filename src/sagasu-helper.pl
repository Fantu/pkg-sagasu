#!/usr/bin/perl -w
# $Id: sagasu-helper.pl,v 1.4 2004/06/12 01:32:10 sarrazip Exp $
# sagasu-helper.pl - Search script for Sagasu - Assumes Latin-1 files
#
# sagasu - GNOME tool to find strings in a set of files
# Copyright (C) 2002-2004 Pierre Sarrazin <http://sarrazip.com/>
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

use strict;
use DirHandle;
use FileHandle;
use locale;
use POSIX qw(locale_h);


# This call allows case insensitive matching ($_ =~ /foo/i) that works
# with Latin-1 accented characters.
#
setlocale(LC_CTYPE, "fr_CA.ISO8859-1");


$| = 1;  # no buffering for STDOUT
my $errPrefix = "*** ";  # string that distinguishes error messages from rest


if (@ARGV != 7 || @ARGV >= 1 && $ARGV[0] eq "--help")
{
    print <<EOF;
Usage: $0 target-expr dir patterns \
		case-sensitive exclude-cvs exclude-symlink-dirs max-depth

'patterns' must be space separated list of file patterns.
'case-sensitive', 'exclude-cvs', and 'exclude-symlink-dirs' must be 1 or 0.
'max-depth' must be the maximum number of subdirectories into which the
recursion is allowed to go.  Zero means search only the current directory.
EOF
    exit 1;
}

my ($targetExpr, $searchDir, $exts) = @ARGV;

# Translate *.cpp to .*\.cpp (for example):
$exts =~ s/\./\\./g;
$exts =~ s/\*/\.\*/g;

my @filePatterns = split(/ +/, $exts);
my $caseSensitive = ($ARGV[3] ne "0");
my $excludeCVSDirs = ($ARGV[4] ne "0");
my $excludeSymlinkDirs = ($ARGV[5] ne "0");
#my $recurseDirs = ($ARGV[4] ne "0");
my $maxDepth = ($ARGV[6] =~ /^(\d+)$/ ? $1 : 0);
$maxDepth = 20 if $maxDepth > 20;

if (0)  # debugging stuff
{
    print "\@ARGV: '", join("|", @ARGV), "'\n";
    print "Searching for: '$targetExpr'\n",
	    "Search directory: '$searchDir'\n",
	    "Patterns of files to search: '",
	    join(",", @filePatterns), "'\n",
	    "Recursion depth: $maxDepth\n";
}

$searchDir = prepareSearchDirName($searchDir);
searchDirTree($targetExpr, $searchDir, \@filePatterns, $caseSensitive, 0);
exit 0;


sub searchDirTree
{
    my ($targetExpr, $searchDir, $raFilePatterns, $caseSensitive, $depth) = @_;

    my $dh = new DirHandle($searchDir);
    if (!defined $dh)
    {
	print "$errPrefix$searchDir: $!\n";
	return;
    }

    my @filenameList = sort $dh->read();
    my @dirList = ();

    # First, search the regular files.
    foreach my $filename (@filenameList)
    {
	next if $filename eq "." || $filename eq "..";
	my $fullname = ($searchDir eq "/" ? "" : "$searchDir") . "/$filename";
	stat($fullname);
	if (-d _)
	{
	    push @dirList, $filename unless
				$filename eq "." || $filename eq "..";
	}
	elsif (-f _ && fileInPatterns($filename, $raFilePatterns))
	{
	    searchFile($targetExpr, $fullname, $caseSensitive);
	}
    }

    # Next, search the subdirectories.
    foreach my $filename (@dirList)
    {
	my $fullname = ($searchDir eq "/" ? "" : "$searchDir") . "/$filename";
	if ($depth < $maxDepth && (!$excludeSymlinkDirs || ! -l $fullname))
	{
	    if (!$excludeCVSDirs || $filename ne "CVS")
	    {
		searchDirTree($targetExpr,
			$fullname, $raFilePatterns, $caseSensitive,
			$depth + 1);
	    }
	}
    }
}


sub searchFile
{
    my ($targetExpr, $searchFilename, $caseSensitive) = @_;

    my $fh = new FileHandle;
    return if !$fh->open($searchFilename);
    my $line;
    if ($caseSensitive)
    {
	while (defined($line = <$fh>))
	{
	    chomp $line;
	    if ($line =~ /$targetExpr/)
	    {
		print "$searchFilename:$.: $line\n";
	    }
	}
    }
    else
    {
	while (defined($line = <$fh>))
	{
	    chomp $line;
	    if ($line =~ /$targetExpr/i)
	    {
		print "$searchFilename:$.: $line\n";
	    }
	}
    }
    $fh->close();
}


sub fileInPatterns
{
    my ($filename, $raFilePatterns) = @_;

    my $ext;
    foreach $ext (@$raFilePatterns)
    {
	return 1 if $filename =~ /^$ext$/;
    }
    return 0;
}


sub prepareSearchDirName
{
    my ($searchDir) = @_;

    if ($searchDir =~ /^~([^\/]*)$/ || $searchDir =~ /^~(.*?)\//)
    {
	my $username = $1;
	if ($username eq "")
	{
	    my $homedir = $ENV{"HOME"};
	    $searchDir =~ s/^~/$homedir/ if (defined $homedir);
	}
	else
	{
	    my @userinfo = getpwnam($username);
	    if (@userinfo >= 8)
	    {
		my $homedir = $userinfo[7];
		$searchDir =~ s/^~$username/$homedir/;
	    }
	}
    }

    return $searchDir;
}
