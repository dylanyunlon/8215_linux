#! /usr/bin/perl

#funtion: generateXml
#purpose: parse excel file, then generate xml file
#by vend_mhf_aesdsrv8@autochips.com 2014/10/24 19:08:50

use bignum;
use strict; 
#use Spreadsheet::ParseExcel;
use Cwd;

my $currentPath = getcwd;
print "current path: ", $currentPath, "\n";
$currentPath .= "/";

use lib $currentPath. "build/tools/PartitionUtility";
use Spreadsheet::ParseExcel;

my $filePath = $currentPath. 'build/tools/Linux_Partition_Table_AC8317.xls';
if($ARGV[0] eq ""){
	$filePath = $currentPath. $ARGV[0];
} else {
	$filePath = $ARGV[0];
}
print "filePath: ", $filePath, "\n";

my $parser = Spreadsheet::ParseExcel->new(); 
my $workbook = $parser->parse($filePath); 

my $result = "<?xml version=". '"1.0"'. " encoding=". '"ISO-8859-1"'. " ?>\n<NFLASH desc=". '"----------  AC8317 EVB Bootup Scatter File  ----------"'.">\n";

if ( !defined $workbook ) { 
        die $parser->error(), ".\n"; 
    } 
print "Open xls Success!\n";

my $sheetname = "nand_ext4";
if($ARGV[3] ne "") {
	$sheetname = "nand_ext4_ab";
}
if($ARGV[2] eq "256") {
	$sheetname = $sheetname. "_256M";
} elsif($ARGV[2] eq "512") {
	$sheetname = $sheetname. "_512M";
} else {
	$sheetname = $sheetname. "_128M";
}

print "sheetname: ", $sheetname, "\n";

my $worksheet = $workbook->worksheet($sheetname);

my $rows = $worksheet->{MaxRow};

print"===== partition parse start =====\n";

my $address = 0;

# 256K align
my $align = 0x40000;

sub align_up {
    my ($addr, $align) = @_;
    return ($addr + $align - 1) & ~($align - 1);
}

for (my $i = 1; $i <= $rows; $i++) {

    my $attri_rw = $worksheet->get_cell($i, 7)->value();
    my $flag     = $worksheet->get_cell($i, 8)->value();
    my $cell     = $worksheet->get_cell($i, 1)->value();

    if ("END" eq $cell) {
        print "==== partition parse end =====\n";
        last;
    }

    print "partition($i): $cell ($attri_rw)\n";

    my $temp = "<partition name=\"" . $cell . "\" ";

    # type
    my $type = $worksheet->get_cell($i, 2)->value();
    $temp .= "type=\"" . $type . "\" ";

    # mount
    my $mount = $worksheet->get_cell($i, 3)->value();
    if ("FALSE" eq $mount) {
        $mount = 0;
    } elsif ("TRUE" eq $mount) {
        $mount = 1;
    }
    $temp .= "mount=\"" . $mount . "\" ";

    # size（KB -> byte）
    my $size = $worksheet->get_cell($i, 4)->value();
    $size *= 1024;

    my $aligned_addr = align_up($address, $align);

    if ($aligned_addr != $address) {
        printf("ALIGN: 0x%X -> 0x%X (gap=0x%X)\n",
            $address,
            $aligned_addr,
            $aligned_addr - $address
        );
    }

    $address = $aligned_addr;

    my $aligned_size = align_up($size, $align);

    if ($aligned_size != $size) {
        printf("SIZE ALIGN: 0x%X -> 0x%X\n", $size, $aligned_size);
    }

    $size = $aligned_size;

    printf("FINAL: start=0x%X size=0x%X\n", $address, $size);

    $temp .= "startaddress=\"0x" . sprintf("%X", $address) . "\" ";
    $temp .= "size=\"0x" . sprintf("%X", $size) . "\" ";

    # image name
    my $img = $worksheet->get_cell($i, 6)->value();

    $temp .= "imagename=\"" . $img . "\" ";
    $temp .= "rw_attribute=\"" . $attri_rw . "\" ";
    $temp .= "flag=\"" . $flag . "\" >\n</partition>\n";

    $result .= $temp;

    $address += $size;
}

$result .="</NFLASH>";

my $filename = $currentPath. "build/tools/scatter.nand.ext4.xml";
if($ARGV[1] ne "") {
	$filename = $ARGV[1]. "scatter.nand.ext4.xml";
}

print "scatter file name: ", $filename, "\n";
open(OUTFILE, "> $filename") or die "Couldn't open $filename for writing: $!";
print OUTFILE $result;
print "write xml file!\n";

close(OUTFILE);
print "close xml file!\n";

print "parse excel file success!\n";
