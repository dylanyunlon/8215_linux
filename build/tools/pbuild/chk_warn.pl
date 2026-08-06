#!/usr/local/bin/perl
###############################################################################
# Copyright Statement:                                                        #
#                                                                             #
#   This software/firmware and related documentation ("MediaTek Software")    #
# are protected under international and related jurisdictions'copyright laws  #
# as unpublished works. The information contained herein is confidential and  #
# proprietary to MediaTek Inc. Without the prior written permission of        #
# MediaTek Inc., any reproduction, modification, use or disclosure of         #
# MediaTek Software, and information contained herein, in whole or in part,   #
# shall be strictly prohibited.                                               #
# MediaTek Inc. Copyright (C) 2010. All rights reserved.                      #
#                                                                             #
#   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND      #
# AGREES TO THE FOLLOWING:                                                    #
#                                                                             #
#   1)Any and all intellectual property rights (including without             #
# limitation, patent, copyright, and trade secrets) in and to this            #
# Software/firmware and related documentation ("MediaTek Software") shall     #
# remain the exclusive property of MediaTek Inc. Any and all intellectual     #
# property rights (including without limitation, patent, copyright, and       #
# trade secrets) in and to any modifications and derivatives to MediaTek      #
# Software, whoever made, shall also remain the exclusive property of         #
# MediaTek Inc.  Nothing herein shall be construed as any transfer of any     #
# title to any intellectual property right in MediaTek Software to Receiver.  #
#                                                                             #
#   2)This MediaTek Software Receiver received from MediaTek Inc. and/or its  #
# representatives is provided to Receiver on an "AS IS" basis only.           #
# MediaTek Inc. expressly disclaims all warranties, expressed or implied,     #
# including but not limited to any implied warranties of merchantability,     #
# non-infringement and fitness for a particular purpose and any warranties    #
# arising out of course of performance, course of dealing or usage of trade.  #
# MediaTek Inc. does not provide any warranty whatsoever with respect to the  #
# software of any third party which may be used by, incorporated in, or       #
# supplied with the MediaTek Software, and Receiver agrees to look only to    #
# such third parties for any warranty claim relating thereto.  Receiver       #
# expressly acknowledges that it is Receiver's sole responsibility to obtain  #
# from any third party all proper licenses contained in or delivered with     #
# MediaTek Software.  MediaTek is not responsible for any MediaTek Software   #
# releases made to Receiver's specifications or to conform to a particular    #
# standard or open forum.                                                     #
#                                                                             #
#   3)Receiver further acknowledge that Receiver may, either presently        #
# and/or in the future, instruct MediaTek Inc. to assist it in the            #
# development and the implementation, in accordance with Receiver's designs,  #
# of certain softwares relating to Receiver's product(s) (the "Services").    #
# Except as may be otherwise agreed to in writing, no warranties of any       #
# kind, whether express or implied, are given by MediaTek Inc. with respect   #
# to the Services provided, and the Services are provided on an "AS IS"       #
# basis. Receiver further acknowledges that the Services may contain errors   #
# that testing is important and it is solely responsible for fully testing    #
# the Services and/or derivatives thereof before they are used, sublicensed   #
# or distributed. Should there be any third party action brought against      #
# MediaTek Inc. arising out of or relating to the Services, Receiver agree    #
# to fully indemnify and hold MediaTek Inc. harmless.  If the parties         #
# mutually agree to enter into or continue a business relationship or other   #
# arrangement, the terms and conditions set forth herein shall remain         #
# effective and, unless explicitly stated otherwise, shall prevail in the     #
# event of a conflict in the terms in any agreements entered into between     #
# the parties.                                                                #
#                                                                             #
#   4)Receiver's sole and exclusive remedy and MediaTek Inc.'s entire and     #
# cumulative liability with respect to MediaTek Software released hereunder   #
# will be, at MediaTek Inc.'s sole discretion, to replace or revise the       #
# MediaTek Software at issue.                                                 #
#                                                                             #
#   5)The transaction contemplated hereunder shall be construed in            #
# accordance with the laws of Singapore, excluding its conflict of laws       #
# principles.  Any disputes, controversies or claims arising thereof and      #
# related thereto shall be settled via arbitration in Singapore, under the    #
# then current rules of the International Chamber of Commerce (ICC).  The     #
# arbitration shall be conducted in English. The awards of the arbitration    #
# shall be final and binding upon both parties and shall be entered and       #
# enforceable in any court of competent jurisdiction.                         #
###############################################################################


#use module
use strict;

our $DEBUG_MODE=0;

main();

sub main()
{
    print "  CHK\tLOG\n";
    
    my ($strLogFile, $strOutFile, $strPrjRoot, $strBranch, $strFindAuthor) = @ARGV;        
    
    if("$strFindAuthor" eq "true")
    {        
        gen_report_with_author($strLogFile, $strOutFile, $strPrjRoot, $strBranch);
    }
    else
    {
        gen_report($strLogFile, $strOutFile, $strPrjRoot);
    }
                                           
}

sub gen_report_with_author
{
    my ($strLogFile, $strOutFile, $strPrjRoot, $strBranch) = @_;
    
    open(IN_FILE, "$strLogFile")  or die("Failed to open file $strLogFile : $!");
    my @aryLogs = <IN_FILE>;
    close (IN_FILE);
    
    my (%symlinkFiles, %fileAuthor);   
    gen_symlink_file_map($strPrjRoot, \%symlinkFiles);
    
    open(OUT_FILE, ">>$strOutFile") or die("Failed to open file $strOutFile : $!");  
    for my $strMsg (@aryLogs)
    {
        chomp($strMsg);        
        if(($strMsg =~ /Warning:/ or $strMsg =~ /warning:/))
        {               
            next , if($strMsg =~ /unavailable:/);
            next , if($strMsg =~ /command-line/);
            next , if($strMsg =~ /mtk_skb_driver/); #open source
        	   my ($strFile, $strSrcFile, $strAuthor);
        	   myPrint(__LINE__, "strMsg == $strMsg");
        	   
            $strFile =`echo "$strMsg" | awk -F : '{print \$1}' `; 
            chomp($strFile);
            $strFile =~ s/.*\/driver\/ko/$strPrjRoot\/chiling\/driver\/ko/;
            myPrint(__LINE__, "strFile == $strFile");
            
            $strSrcFile = get_link_src_file($strFile, \%symlinkFiles);           
            $strSrcFile =~ s/$strPrjRoot\///;
            myPrint(__LINE__, "strSrcFile == $strSrcFile");
            
            my $strP4Path = get_depot_path($strBranch);
            
            if(defined($fileAuthor{"$strSrcFile"}))
            {
                $strAuthor = $fileAuthor{"$strSrcFile"}	;
            }
            else
            {
                $strAuthor = get_author($strSrcFile, $strP4Path, \%fileAuthor);
            }
            myPrint(__LINE__, "strAuthor == $strAuthor");
            print OUT_FILE "$strAuthor : $strMsg\n";
        }
    }
    close(OUT_FILE);
}

sub gen_report
{
    
}

sub get_link_src_file
{
    my ($strFile, $symlinkFiles) = @_;
    
    my $strLinkSrcFile = $strFile;  
    #2. symlink file    
    for(my $i = 0; $i < 5; $i++)
    {
        foreach my $strLink (sort keys %{$symlinkFiles})
        {            
            chomp($strLink);
            if($strLinkSrcFile =~ /$strLink/)
            {
                $strLinkSrcFile =~ s/$strLink/$symlinkFiles->{$strLink}/;    
            }
        }
    }    
    
    return $strLinkSrcFile;
}

sub get_depot_path
{
    my ($strBranch) = @_;
    myPrint(__LINE__, "Get depot path ... ");
    my $strP4Path; 
    my $str2ndPath = `p4 dirs //DTV/*`;	
    my @ary2ndPath = split /\n/, $str2ndPath; 
    foreach my $strP1 (@ary2ndPath)
    {
        my $strPath = `p4 dirs $strP1/$strBranch 2>&1`;
        if ($strPath !~/no such file/)
    	   {
    		      $strP4Path = $strPath;
    		      chomp($strP4Path);
    		      last;
        }
    }
    return $strP4Path;
}

sub get_author
{
    my ($strFile, $strP4Path, $fileAuthor) = @_;
    myPrint(__LINE__, "Find author ... ");
    my $strAuthor = "";
    if(defined($strFile) and defined($strP4Path))
    {
    	   $strAuthor = `p4 changes -t -m1  "$strP4Path/$strFile" | awk '{print \$7}' | sed -e 's/\@.*//g' `;
        chomp($strAuthor);
        if($strAuthor eq "dtvbm11" and $strP4Path ne "//DTV/PROD_BR/DTV_X_IDTV0801")
        {
            #get author from //DTV/PROD_BR/DTV_X_IDTV0801
            $strAuthor = `p4 changes -t -m1  "//DTV/PROD_BR/DTV_X_IDTV0801/$strFile" | awk '{print \$7}' | sed -e 's/\@.*//g' `;
            chomp($strAuthor);
        }
        if("$strAuthor" eq "luis.chen")
        {
            $strAuthor = "joshua.huang";
        }
        $fileAuthor->{"$strFile"} = $strAuthor;
        return $strAuthor;
    }
}

sub gen_symlink_file_map
{
    my ($strPrjRoot, $symlinkFiles) = @_;
    
    myPrint(__LINE__, "Generate symbolic link files");
    
    my @aryLinkFiles = `find $strPrjRoot -type l | grep -v "\.so"`;
    
    foreach my $strLinkFile (@aryLinkFiles)
    {
        chomp($strLinkFile);
        #print "$strLinkFile \n";
        my $strSrcFile = `readlink "$strLinkFile"`;
        chomp($strSrcFile);
        $symlinkFiles->{"$strLinkFile"} = "$strSrcFile";
    }
}

sub myPrint
{
    my $strLine        = $_[0];
    my $strString      = $_[1];
    print "$strLine>> $strString\n" if ($DEBUG_MODE);
}

sub chk_P4_status
{
    my $msg = `p4 set 2>&1`;
    return 0 if ($msg =~ /command not found/);
    return 0 if ($msg =~ /cannot execute binary file/);
    my $msg2 = `p4 info 2>&1`;
    return 0 if ($msg2 =~ /Perforce client error/);

    my $strPath = `pwd`;
    chomp($strPath);
    return 0 if ($strPath =~ /\/release\//);
 
    return 1;
}