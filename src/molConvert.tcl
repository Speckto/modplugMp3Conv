#!/bin/sh
#-*-tcl-*-
# the next line restarts using tclsh \
exec tclsh "$0" -- ${1+"$@"}
###############################################################################
#
# Converts .MOL (Module Lists) to alternative playlist formats.
# MOL files are used by MOD4WIN and the modplug player to save playlists.
#
# Since mol files appear to be used only on windows this script may not
# work on linux/unix..
#
#
###############################################################################

###############################################################################
proc Usage { errMsg } {
    if {[string length $errMsg] > 0} {
        puts "$errMsg"
    }
    puts ""
    puts "molConvert - Convert Mol (Module Playlists) to other formats"
    puts "USAGE:"
    puts "   molConvert options <infile.mol> <outfile>"
    puts ""
    puts "Options:"
    puts " --outputformat Specifies output format to use for outfile"
    puts "                Defaults to TXT format"
    puts "                Supported formats:"
    puts "                 TXT - Plain text file listing"
    puts " --verifyfiles  Forces verification that files in output file exist"
    puts ""
}

###############################################################################
# Each line in the [FILES] section of the MOL file describes a single playlist
# entry.
# <fileext><dirindex><1E><space><filename><eol>
# 
# <fileext>  is a value (see ModExtensionXRef array) describing the file 
#            extension
# <dirindex> Is a character indicating the directory path in which the file
#            is located (in the [DIRECTORIES] section). Indexing starts with
#            the space character (ascii 32)
# <filename> File name usually excluding the file extension.
#            If the file extension is not supported by <fileext> then
#            the full filename (no path) is stored
#
# You may need to add new entries for these depending on your playlist
# I only had these file types in my system.
set ModExtensionXRef(\$) "MOD"
set ModExtensionXRef(\/) "XM"
set ModExtensionXRef(\-) "S3M"
set ModExtensionXRef(3)  "IT"

###############################################################################
set mcglobals(options,platform)       "LINUX"
set mcglobals(options,debugging)      0
set mcglobals(options,outputformat)   "TXT"
set mcglobals(options,verifyfiles)    0
set mcglobals(options,filepaths)      [list]

set mcglobals(conversion,dirs)             [list]
set mcglobals(conversion,outFileListing)   [list]

###############################################################################

proc ConvertPlaylist { } {
    global mcglobals
    set fp $mcglobals(options,filepaths)
    set mcglobals(options,filepaths) [list]
    foreach f $fp {
        lappend mcglobals(options,filepaths) [file normalize $f]
    }

    set inFileName  [lindex $mcglobals(options,filepaths) 0]
    set outFileName [lindex $mcglobals(options,filepaths) 1]

    set mcglobals(options,inFileDrive)\
        [string range $inFileName 0 1]

    if {[catch {set inputFile [open $inFileName r]}]} {
        fatal-error "Couldn't open input file '$inFileName'"
    }

    if {[catch {set outputFile [open $outFileName w]}]} {
        fatal-error "Couldn't open file '$outFileName'"
    }

    # Now we can read in the source file
    ReadSourceFile $inFileName $inputFile

    # Write out
    puts "Converting: $inFileName"
    puts "Writing converted playlist to: $outFileName"
    foreach f $mcglobals(conversion,outFileListing) {
        puts $outputFile "$f"
    }
    close $inputFile
    close $outputFile
}

###############################################################################
proc ReadSourceFile {inFileName inputFile} {
    global mcglobals
    set retVal 1
    # -1= error
    # 0 = Initial read
    # 1 = Reading directories
    # 2 = Skipping section
    # 3 = Reading files
    set state 0
    while {($state!=-1) && (![eof $inputFile]) } {
        if {([gets $inputFile line]) && \
                ([string length $line] > 1)} {

            switch $state {
                0 {
                    if {[string compare $line "\[DIRECTORIES\]"] == 0} {
                        set state 1
                    } else {
                        set state -1
                        fatal-error "Input file has incorrect format"
                       
                    }
                }
                1 {
                    if { [string index $line 0] == "\[" } {
                        if {[string compare $line "\[FILES\]"] == 0} {
                            set state 3
                        } else {
                            set state 2
                        }
                    } else {
                        lappend mcglobals(conversion,dirs) \
                            [string trim $line " "]
                    }
                }
                2 {
                    if {[string compare $line "\[FILES\]"] == 0} {
                        set state 3
                    } else {
                        set state 2
                    }
                }
                3 {
                    if { [string index $line 0] != "\[" } {
                        ConvertLine $line
                    } else {
                        if {[string compare $line "\[FILES\]"] == 0} {
                            set state 3
                        } else {
                            set state 2
                        }
                    }
                }
            }

        } ; # gets
    } ; # while

    if {$state == -1} {
        set retVal 0
    }
    return $retVal
}

###############################################################################
proc ConvertLine { line } {
    global mcglobals
    global ModExtensionXRef
    set fileExtChar  [string index $line 0]
    set dirIndexChar [string index $line 1]
    set fnamebit     [string range $line 4 end]

    # Map ext char
    set fileExtension ""
    if {[info exists ModExtensionXRef($fileExtChar)]} {
        set fileExtension $ModExtensionXRef($fileExtChar)
    }

    debug-puts "$fnamebit : EXTCHAR:\"$fileExtChar\" MAPPED:\"$fileExtension\""

    # Map dir Index
    set directoryPath ""
    scan $dirIndexChar "%c" numericIndexC
    set numericIndex [expr ($numericIndexC - 32)]
    debug-puts "$fnamebit : CHAR:\"$dirIndexChar\" CODE:$numericIndexC INDEX:$numericIndex"
    if {$numericIndex < [llength $mcglobals(conversion,dirs)] } {
        set directoryPath [lindex \
                               $mcglobals(conversion,dirs) \
                               $numericIndex]
    } else {
        puts "Line is \"$line\""
        puts "Index of directory: $numericIndex"
        set i 0
        foreach f $mcglobals(conversion,dirs) {
            puts "DIR $i: $f"
            incr i
        }
        fatal-error "Conversion failed - file entry specifies unknown directory"
    }
    
    # Join it all up
    set fullFilePath ""
    append fullFilePath $directoryPath
    set fullFilePath [file join $fullFilePath $fnamebit]
    if {$fileExtension != "" } {
        append fullFilePath ".$fileExtension"
    }

    # Windows specific code - we need to prepend a drive label - if there is
    # not one there already
    # Also need to be network path aware (e.g. starts with \\)
    set first2Chars [string range $fullFilePath 0 1]
    if {[string compare $first2Chars "\\\\"] != 0} {
        # Not a network path
        # check for [A-Za-z]:
        set regRet [regexp -- {[A-Za-z]:} $first2Chars match]
        if {!$regRet} {
            set fullFilePath \
                "${mcglobals(options,inFileDrive)}${fullFilePath}"
        }
    }

    lappend mcglobals(conversion,outFileListing) $fullFilePath
}

###############################################################################
proc VerifyFiles { } {
    global mcglobals
    puts "Verifying output files exist.."
    foreach f $mcglobals(conversion,outFileListing) { 
        if {![file exists $f]} { 
            puts "Missing file: $f"
        }
    }
}
###############################################################################
proc fatal-error {msg} {
    puts "fatal-error ($msg)"
    exit 2
}

proc debug-puts { msg } {
    global mcglobals
    if {$mcglobals(options,debugging) == 1} {
        puts "debug-msg: $msg"
    }
}
###############################################################################
proc ParseArgs { argc argv } {
    global mcglobals
    set parsedOK 1

    set argindex 0
    while {$argindex < $argc} {
        set arg [lindex $argv $argindex]
        switch -regexp -- $arg {
            "^-h" -
            "^--help" {
                Usage ""
                exit 0
            }
            "^--outputformat$" {
                incr argindex
                set mcglobals(options,outputformat) [lindex $argv $argindex]
            }
            "^--verifyfiles$" {
                set mcglobals(options,verifyfiles) 1
            }
            default {
                lappend mcglobals(options,filepaths) $arg                    
            }
        }
        incr argindex
    }

    if {[llength $mcglobals(options,filepaths)] == 2} {
        if {![file exists [lindex $mcglobals(options,filepaths) 0]]} {
            set parsedOK 0
            Usage "Input file does not exist: [lindex $mcglobals(options,filepaths) 0]"
            exit 0
            
        }
    } else {
        set parsedOK 0
        Usage "Unexpected parameter:[lindex $mcglobals(options,filepaths) end]"
        exit 0
    }
    return $parsedOK
}

###############################################################################
proc Init { } {
    global env mcglobals
    if {[info exists env(OS)]} {
        set pl [string tolower $env(OS)]
        if {[string first "windows" $pl] > -1} {
            set mcglobals(options,platform) "WINDOWS"
        }
    }
    puts "Detected platform: $mcglobals(options,platform)"
}

###############################################################################
proc main { argc argv } {
    global env mcglobals
    if {$argc > 1} {
        # Need in and out file names
        ParseArgs $argc $argv
        Init
        ConvertPlaylist
        if {$mcglobals(options,verifyfiles) == 1} {
            VerifyFiles
        }
    } else {
        Usage "Missing arguments"
    }
}

###############################################################################
main $argc $argv