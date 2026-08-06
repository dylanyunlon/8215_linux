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

#my $filePath = $currentPath. "build/tools/Linux_Partition_Table_AC8317.xls";
my $filePath = "";
if($ARGV[0] eq ""){
	$filePath = $currentPath. "build/tools/Linux_Partition_Table_AC8317.xls";
} else {
	$filePath = $ARGV[0];
}
print "filePath: ", $filePath, "\n";

my $parser = Spreadsheet::ParseExcel->new(); 
my $workbook = $parser->parse($filePath); 

my $result = "<?xml version=". '"1.0"'. " encoding=". '"ISO-8859-1"'. " ?>\n<NFLASH desc=". '"----------  AC8317 EVB Bootup Scatter File  ----------"';

if ( !defined $workbook ) { 
        die $parser->error(), ".\n"; 
    } 
print "Open xls Success!\n";

my $sheetname = "emmc";
if($ARGV[2] ne "") {
	$sheetname = "emmc_ab";
}
print "sheetname: ", $sheetname, "\n";

my $worksheet = $workbook->worksheet($sheetname);
my $size = 0x0;
my $startaddress = 0x0;
my $rows = $worksheet->{MaxRow};

print"=====get emmc write protect group(wpg) size====\n";
my $wpg_size = 0;
for(my $i=1;$i<$rows;$i++)
{
	my $fcell = $worksheet->get_cell($i,1);
	last unless $fcell;
	my $cell_val = $fcell->value();

	if("wpg_size(MB)" eq $cell_val)
	{
		$wpg_size = $worksheet->get_cell($i,2)->value();
		print "[wp]get configed wpg_size(MB): ", $wpg_size, "\n";
		if($wpg_size%4 != 0)
		{
			print "ERROR:[wp] wpg_size is not power of 4, set wpg_size to 0\n";
			$wpg_size = 0;
		}
		last;
	}
}
$result .= ' wpg_size="'.$wpg_size.'"'.">\n";
$wpg_size *=1024;
$wpg_size *=1024; # MB to Byte
print"[wp]:wpg_size = 0x", sprintf("%X", $wpg_size), "\n";

print"===== partition parse start =====\n";

my $flag_wp_start = "no";
for(my $i=1;$i<$rows;$i++)
{
my $attri_rw = $worksheet->get_cell($i,7)->value();
my $flag = $worksheet->get_cell($i,8)->value();
my $cell = $worksheet->get_cell($i,1)->value();
my $wp_flag = $flag&0x10;

	#print"\n\n####",$cell,"####\n";
	
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
	#print "size= 0x", sprintf("%x",$size),"\n";
	$startaddress += $size;

	if($wpg_size != 0) {
		if($wp_flag == 16)
		{
			if("no" eq $flag_wp_start)
			{
				print"[wp]++++got wp region start address: 0x", sprintf("%X", $startaddress), "\n";
				$flag_wp_start = "yes";
				if($startaddress % $wpg_size != 0)
				{
					$startaddress += $wpg_size - ($startaddress % $wpg_size);
					print"[wp]    start: align to 0x", sprintf("%X", $startaddress), "\n";
				}

			}
		}
		else
		{
			if("yes" eq $flag_wp_start)
			{
				print"[wp]----got wp region end address: 0x", sprintf("%X", $startaddress), "\n";
				$flag_wp_start = "no";

				if($startaddress % $wpg_size != 0)
				{
					$startaddress += $wpg_size - ($startaddress % $wpg_size);
					print"[wp]    end:  align to 0x", sprintf("%X", $startaddress), "\n";
				}
			}
		}
	}

	$size = $cell;
	$temp .= 'startaddress="0x'. sprintf("%X", $startaddress). '" size="0x'. sprintf("%X",$size). '" ';

	$cell = $worksheet->get_cell($i,6)->value();
	$temp .= 'imagename="'. $cell. '" rw_attribute="'. $attri_rw. '" flag="'. $flag. '" >'. "\n". " </partition>". "\n";
	
	#print $temp;
	$result .= $temp;
}

$result .="</NFLASH>";

my $filename = $currentPath. "build/tools/scatter.mmcboot.ext4.xml";
if($ARGV[1] ne "") {
	$filename = $ARGV[1]. "scatter.mmcboot.ext4.xml";
}

print "scatter file name: ", $filename, "\n";
open(OUTFILE, "> $filename") or die "Couldn't open $filename for writing: $!"; 
print OUTFILE $result;
print "write xml file!\n";

close(OUTFILE);
print "close xml file!\n";

print "parse excel file success!\n";
