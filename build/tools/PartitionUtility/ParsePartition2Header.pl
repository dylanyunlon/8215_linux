#! /usr/bin/perl

#funtion: generate_partition_header
#purpose: parse excel file, then generate a header file include partition
#modified by qingqi.xia@autochips.com 2014/10/24 19:08:50

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

my $result = "#ifndef _ATC_PARTITION_H_\n#define _ATC_PARTITION_H_\n\n";

if ( !defined $workbook ) { 
        die $parser->error(), ".\n"; 
    } 
print "Open xls Success!\n";
my $sheetname = "nand_ext4";
if($ARGV[2] ne "") {
	$sheetname = $ARGV[2];
}
print "sheetname: ", $sheetname, "\n";
my $worksheet = $workbook->worksheet($sheetname);

my $adress = 0x0;
my $size = 0x0;
my $startadress = 0x0;

my $rows = $worksheet->{MaxRow};
my $part_num = 0;

# ========== Define Partition Info Strut ===========
$result .= 'struct atc_part_info {'."\n"; 
$result .= "\t".'char part_name[32];'."\n"; 
$result .= "\t".'char part_type[16];'."\n"; 
$result .= "\t".'u8 mount_flag;'."\n"; 
$result .= "\t".'u64 part_offset;'."\n"; 
$result .= "\t".'u64 part_size;'."\n"; 
$result .= "\t".'char rw_type[3];'."\n";
$result .= '};'."\n\n"; 

# ========== Get Partition Number ===========
for(my $j=1; $j<$rows; $j++)
{
	my $cell = $worksheet->get_cell($j,1)->value();
	if("END" eq $cell){
		last;
	}
	$part_num += 1;
}
$result .= "#define PART_NUM\t\t\t(".$part_num. ")\n\nstatic const struct atc_part_info PartInfo[PART_NUM]={\n";

print"===== partition parse start =====\n";

for(my $i=1;$i<$rows;$i++)
{
	my $cell = $worksheet->get_cell($i,1)->value();
	
	if("END" eq $cell)
	{
		print"===== partition parse end =====\n";
		last;
	}
	print "partitiion(", $i, "): ", $cell, "\n";
	
	# Partition Name
	my $temp = "\t\t\t".'{ "'. $cell. '", ';
	
	# Partition Type
	$cell = $worksheet->get_cell($i,2)->value();
	$temp .= '"'. $cell. '", ';

  # Mount Type
	$cell = $worksheet->get_cell($i,3)->value();
	if('FALSE' eq $cell)
	{
		$cell = 0;
	}elsif('TRUE' eq $cell)
	{
		$cell = 1;
	}
	$temp .=$cell.', ';

	
	$cell = $worksheet->get_cell($i,4)->value();
	$cell *=1024;
	#print "cell= ", $cell, "\n";
	#print "size= ", sprintf("%x",$size),"\n";
	#print "adress= ", $adress,"\n";
	$startadress = sprintf("%X",$adress + $size);
	#print "startadress= ", $startadress, "\n";
	$adress = hex($startadress);
	#print "startadress= ", $startadress, "\n";
	$size = $cell;
	$temp .= '0x'. $startadress. ', 0x'. sprintf("%X",$size). ', ';

	$cell = $worksheet->get_cell($i,7)->value();
	$temp .= '"'.$cell.'"},'."\n";

	#$cell = $worksheet->get_cell($i,6)->value();
	
	#print $temp;
	
	$result .= $temp;
	#print "Value =", $cell, "\n";
}

$result .="};\n\n#endif /*_ATC_PARTITION_H_*/";
my $filename = $ARGV[1];

print "Header file name: ", $filename, "\n";
open(OUTFILE, "> $filename") or die "Couldn't open $filename for writing: $!"; 
print OUTFILE $result;
print "Write header file!\n";

close(OUTFILE);
print "close header file!\n";

print "parse excel file success!\n";
