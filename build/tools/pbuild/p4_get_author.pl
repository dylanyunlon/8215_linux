#!/usr/bin/perl
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
#
# Author: Chien-Ping Cheng (cp_cheng\@mtk.com.tw) on 2004-11-08
#
# Description:
# This script will
# 1. auto_build and send email
#***********************************************************************
#use module
use strict;
use Getopt::Std;
#use MIME::Lite; #for e-mail utility

#show title
my $TITLE = "
========================================================================
 [Program]          p4_get_author.pl
 [Version]          V1.0
 [Revision Date]    2009-04-02
 [Author]           Brianpc Huang, brianpc_huang\@mediatek.com, 21895, 2009-04-02
 [Modified]         Brianpc Huang,
 [Last Update]      2007-03-29
 [Copyright]
    Copyright (C) 2004 MediaTek Incorporation. All Rights Reserved.
========================================================================
";

my @aryExcludeList = ("project_x/middleware/inet/openssl");

my $strFindAuthor="false";
my $strCheckWarning="false";
my $DEBUG_MODE =0;
my @aryAuthor;

my %symlinkFiles;
my $strPrjRoot = "";
my $strP4Path ;
my %fileAuthor;

main();

sub main
{
    
    print "  CHK\tLOG\n";
    my $strBranch = "";
    my $strWarnErrFile = "";
    my $strErrFile ="";
    my $strWarningFile ="";
        
    ($strPrjRoot, $strBranch, $strWarnErrFile, $strErrFile, $strFindAuthor, $strCheckWarning) = @ARGV;
    
    die "Not find Branch or WarnErrFile" if (($strBranch eq "") or ($strWarnErrFile eq "") or ($strErrFile eq ""));
    $strWarningFile = $strErrFile ;
    $strWarningFile =~ s/_fail/_warning/g;
	
	
    my @aryMsg;
    my @aryFullMsg;
	
    if (! chk_P4_status() or $strFindAuthor eq "false" )
    {
        # generate Warning/Error message without author
        @aryMsg = gen_report($strWarnErrFile,\@aryFullMsg);
    }
    else
    {
        # generate Warning/Error message with author
        @aryMsg = gen_report_w_author($strBranch,$strWarnErrFile,\@aryFullMsg);
    }

    print "\n========== Build Result Info  Branch = $strBranch ===============\n" if (scalar @aryMsg >0);
    for(my $i = 0; $i < scalar @aryMsg; $i++) 
    {
        my $strMsg = @aryMsg[$i];
        my $Fullmsg = @aryFullMsg[$i];
        my $strAuthor = @aryAuthor[$i];
        
        my $msg = $strMsg ;
        $msg =~ s/\(/\\\(/g;
        $msg =~ s/\)/\\\)/g;
        
        $msg =~ s/;;;/, /g;
        $Fullmsg =~ s/;;;/, /g;
        
        
        if ( ($msg =~ /Error/) or ($msg =~ /error:/) or ($msg =~ /FAIL,/))
        {
            putLog($strErrFile, "$strAuthor, $Fullmsg");
        }
        else
        {
            putLog($strWarningFile, "$strAuthor, $Fullmsg");
        }
        if ($strAuthor ne '')
        {
            print "$strAuthor, ";
        }
        print "$msg\n";
    }
    print "\n\n" if (scalar @aryMsg > 0);
	
    my $strRst = join ("\n", @aryMsg);
    return 0;
	
}

sub gen_report_w_author
{
    my ($strBranch, $strWarnErrFile, $argFullMsg) = @_;
    
    gen_symlink_file_list(), if ($strFindAuthor eq "true");
    
    $strP4Path = p4_depot_path($strBranch), if ($strFindAuthor eq "true");
    
    my @aryMsg = gen_report($strWarnErrFile,$argFullMsg);
    my @aryRstMsg;
    foreach my $strMsg (@aryMsg) 
    {       
    	   my $strAuthor = "";
    	   my ($strType, $strAbsPath, $msg);

        if($strMsg =~ /(.*):\s+(.*;;;.*;;;.*)/)
        {
            
            $strAuthor = $1;
            $strMsg = $2;
        }
        else
        {
            ($strType, $strAbsPath, $msg) = $strMsg =~ m/(.*);;;(.*);;;(.*)/;
            $strAuthor = get_author($strAbsPath) if ($strFindAuthor eq "true");            
        }
        push (@aryRstMsg,"$strMsg");
        push (@aryAuthor, $strAuthor);
    }
    
    return @aryRstMsg;
}

sub get_author
{
    my ($strFile) = @_;
    my $strAuthor ;
    if(defined($fileAuthor{"$strFile"}))
    {
        $strAuthor = $fileAuthor{"$strFile"}	;
    }
    else
    {
        $strAuthor = p4_file_author($strFile);
    }    
    
    return $strAuthor ;
}

sub p4_file_author
{
    my ($strFile) = @_;
    myPrint(__LINE__, "Find author ... ");
    my $strAuthor ;
    
    if(defined($strFile) and defined($strP4Path))
    {
    	   $strAuthor = `p4 filelog -m 1 -s $strP4Path/$strFile 2>/dev/null|head -n 2|tail -n 1 | awk '{print \$9}' |sed  's/\@.*//g'`;
        chomp($strAuthor);
        $strAuthor = "super" , if ($strAuthor eq "");
        $fileAuthor{"$strFile"} = $strAuthor;
        return $strAuthor;
    }
    $strAuthor = "super";
    return $strAuthor ;
}

sub p4_depot_path
{
    my ($strBranch) = @_;
    myPrint(__LINE__, "Get depot path ... ");
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

sub gen_report
{
    my ($strWarnErrFile,$argFullMsg) =@_;
    
	   die "$strWarnErrFile does not exist", if (! -e $strWarnErrFile);
	   
    my @aryPath;
    my @aryMsgList;
    my $strFindFile = "";
    my $strType;
    
    my ($strFileC, $strFileO) ;
    
    open(IN_FILE, "<$strWarnErrFile")  or die("Failed to open file $strWarnErrFile.");
    my @aryLogs = <IN_FILE>;
    close (IN_FILE); 
    my $CHECK_PHASE2 = "false";
    if(`head -n 100 $strWarnErrFile | grep -E ".+,.+\.o\.log" | wc -l ` != 0)
    {
        $CHECK_PHASE2 = "true";
    }
    
    foreach my $strLog (@aryLogs)
    {    	   
        chomp($strLog);  
        my $strMsg;
        if("$CHECK_PHASE2" eq "true")
        {
            ($strFileC, $strFileO) = $strLog =~ m/(.*),(.*)/;
            $strFileO = trim($strFileO);
            myPrint( __LINE__ ,"strFileC:\t$strFileC");
            myPrint( __LINE__ ,"strFileO:\t$strFileO");
            my $nSizeFileO = -s $strFileO;
            if ($nSizeFileO != 0)    
            {
                $strMsg  =`sed ':a; /\$/N; s/\\n//; ta' $strFileO`;
            }   
            else
            {
                next;
            }                

        }
        else
        {
            if($strLog =~ /Error:/ or $strLog =~ /error:/ or $strLog =~ /Warning:/ or $strLog =~ /warning:/ or $strLog =~ /multiple definition of/ or $strLog =~ /Undefined Symbol:/)
            {
            	   next , if($strLog =~ /FAIL,/);
                next , if($strLog =~ /jobserver unavailable/);
                next , if($strLog =~ /command-line/);
                next , if($strLog =~ /overriding commands/);
                next , if($strLog =~ /warning: ignoring old commands/);
                next , if($strLog =~ /ld: warning:/);          
                next , if($strLog =~ /ko\] undefined/);             
                $strFileC =`echo "$strLog" | awk -F : '{print \$1}' `;         
                chomp($strFileC);
                $strFileC =~ s/.*\/driver\/ko/$strPrjRoot\/chiling\/driver\/ko/;      
                $strFileC =~ s/.*\/chiling\/kernel/$strPrjRoot\/chiling\/kernel/; 
                $strFileC =~ s/Source\/DirectFB/$strPrjRoot\/chiling\/driver\/directfb\/Source\/DirectFB/;  
                $strMsg = $strLog; 

            } 
            else
            {
                next;
            }            
        }

        if($strMsg =~ /.*;;;.*;;;.*/)
        {
            push (@aryMsgList, $strMsg);
            push (@$argFullMsg, $strMsg);
            next;
        }
        
        $strMsg =~ s/\s+/ /g;
        $strType = "Warn" if (($strMsg =~ /\bWarn/) or ($strMsg =~ /\bwarning/) );
        $strType = "FAIL" if ((($strMsg =~ /(\bError|\berror)/) and ($strMsg !~ / 0 errors/)) or (($strMsg =~ /multiple definition of/) and ($strMsg !~ / 0 errors/)));

        next , if("$strCheckWarning" ne "true" and "$strType" eq "Warn" );
                
        $strFileC = get_link_src_file($strFileC); 
        
        my ($strVm,$strPrjPath) = $strFileC =~ m/(vm_linux|vm_proj_x)\/(.*)/;

        my $fragment =  substr ($strMsg, 0, 2048);
        my $large_fragment =  substr ($strMsg, 0, 2048);
        
        myPrint( __LINE__ ,"strMsg:\t$strMsg");
        
        my $msg ="$strType;;;$strPrjPath;;;\"$fragment\"";
        my $strFullMsg ="$strType;;;$strPrjPath;;;\"$large_fragment\"";
        myPrint( __LINE__ ,"msg:\t$msg");
        push (@aryMsgList, $msg);
        push (@$argFullMsg, $strFullMsg);   
    }
    
    return @aryMsgList;

}

sub gen_symlink_file_list
{
    myPrint(__LINE__, "Generate symlink files");
    
    my @aryLinkFiles = `find $strPrjRoot -type l | grep -v "\.so"` ;
    
    foreach my $strLinkFile (@aryLinkFiles)
    {
        chomp($strLinkFile);
        #print "$strLinkFile \n";
        my $strSrcFile = `readlink "$strLinkFile"`;
        chomp($strSrcFile);
        $symlinkFiles{"$strLinkFile"} = "$strSrcFile";
    }
}

sub get_link_src_file
{
    my ($strFile) = @_;
    
    my $strLinkSrcFile = $strFile;  

    #2. symlink file    
    for(my $i = 0; $i < 5; $i++)
    {
        foreach my $strLink (sort keys %symlinkFiles)
        {            
            chomp($strLink);
            if($strLinkSrcFile =~ m/\Q$strLink\E/)
            {
                $strLinkSrcFile =~ s/$strLink/$symlinkFiles{$strLink}/;    
            }
        }
    }    
    return $strLinkSrcFile;
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

sub myPrint
{
    my $strLine        = $_[0];
    my $strString      = $_[1];
    print "$strLine>> $strString\n" if ($DEBUG_MODE);
}

sub like_array
{
    my $ary    = $_[0];
    my $target = $_[1];
    
    for(my $i = 0; $i < scalar @$ary; $i++) 
    {
        my $src = @$ary[$i];
        $src =~ s/\//\\\//;
        return 1 if ($target =~/@$ary[$i]/);
    } 
    return 0;
}

sub trim
{
    my $strString = @_[0];
    for ($strString) 
    {   
        # trim white space in $variable, cheap
        s/^\s+//;
        s/\s+$//;  
    }
    return $strString;
} # end of sub



sub putLog
{
    my ($strFile, $strMsg) = @_;
    open(LOG_FILE, ">>$strFile") or myDie(__LINE__,"Failed to open file $strFile .");
    print LOG_FILE $strMsg;
    print LOG_FILE "\n";
    close LOG_FILE;
}
