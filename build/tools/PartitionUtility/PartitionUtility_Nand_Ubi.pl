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
if($ARGV[0]){
	$filePath = $currentPath. $ARGV[0];
}
print "filePath: ", $filePath, "\n";

my $parser = Spreadsheet::ParseExcel->new(); 
my $workbook = $parser->parse($filePath); 

my $result = "<?xml version=". '"1.0"'. " encoding=". '"ISO-8859-1"'. " ?>\n<NFLASH desc=". '"----------  AC8317 EVB Bootup Scatter File  ----------"'.">\n";

if ( !defined $workbook ) { 
        die $parser->error(), ".\n"; 
    } 
print "Open xls Success!\n";

my $worksheet = $workbook->worksheet('nand_ubi');

my $adress = 0x0;
my $size = 0x0;
my $startadress = 0x0;

my $rows = $worksheet->{MaxRow};

print"===== partition parse start =====\n";

for(my $i=1;$i<$rows;$i++)
{
my $attri_rw = $worksheet->get_cell($i,7)->value();
my $cell = $worksheet->get_cell($i,1)->value();
	
	if("END" eq $cell)
	{
		print"===== partition parse end =====\n";
		last;
	}
	print "partitiion(", $i, "): ", $cell, "  (", $attri_rw, ")\n";
	
	my $temp = ' <partition name="'. $cell. '"   ';
	
	$cell = $worksheet->get_cell($i,2)->value();
	$temp .= 'type="'. $cell. '" ';

	$cell = $worksheet->get_cell($i,3)->value();
	if('FALSE' eq $cell)
	{
		$cell = 0;
	}elsif('TRUE' eq $cell)
	{
		$cell = 1;
	}
	$temp .='mount="'. $cell.'" ';

	
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
	$temp .= 'startaddress="0x'. $startadress. '" size="0x'. sprintf("%X",$size). '" ';

	$cell = $worksheet->get_cell($i,6)->value();
	$temp .= 'imagename="'. $cell. '" rw_attribute="'. $attri_rw. '" >'. "\n". " </partition>". "\n";
	
	#print $temp;
	
	$result .= $temp;
	#print "Value =", $cell, "\n";
}

$result .="</NFLASH>";

my $filename = $currentPath. "build/tools/scatter.nand.ubi.xml";

print "scatter file name: ", $filename, "\n";
open(OUTFILE, "> $filename") or die "Couldn't open $filename for writing: $!"; 
print OUTFILE $result;
print "write xml file!\n";

close(OUTFILE);
print "close xml file!\n";

print "parse excel file success!\n";
