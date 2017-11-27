#!/usr/bin/python  

import sys, string, locale
from optparse import OptionParser

delimiter = '\t'

def output(lines1, lines2, one, two, three):

    len1 = len(lines1)
    len2 = len(lines2)
    i = 0
    j = 0
    oneUnsorted = True
    twoUnsorted = True
    '''this determines how many tabs is required to move each col'''
    colTwoTab = 0
    colThreeTab = 0
    if not one:
        colTwoTab=colTwoTab+1
        colThreeTab=colThreeTab+1
    if not two:
        colThreeTab=colThreeTab+1
        
    while i < len1 and j < len2:

        if lines1[i] == lines2[j]:
            if not three:
                sys.stdout.write(delimiter*colThreeTab + lines1[i])
            j=j+1
            i=i+1
            continue
        elif lines1[i] < lines2[j]:
            if i > 0 and (lines1[i-1] > lines1[i]) and oneUnsorted:
                sys.stderr.write("comm: file 1 is not in sorted order\n")
                oneUnsorted = False
            if not one:
                sys.stdout.write(lines1[i])
            i=i+1
            continue
        elif lines2[j] < lines1[i]:
            if j > 0 and (lines2[j-1] > lines2[j]) and twoUnsorted:
                sys.stderr.write("comm: file 2 is not in sorted order\n")
                twoUnsorted = False
            if not two:
                sys.stdout.write(delimiter*colTwoTab + lines2[j])
            j=j+1
            continue

    if not one:
        while i < len1:
            if i > 0 and (lines1[i-1] > lines1[i]) and oneUnsorted:
                sys.stderr.write("comm: file 1 is not in sorted order\n")
                oneUnsorted = False
            sys.stdout.write(lines1[i])
            i=i+1
    if not two:
        while j < len2:
            if j > 0 and (lines2[j-1] > lines2[j]) and twoUnsorted:
                sys.stderr.write("comm: file 2 is not in sorted order\n")
                twoUnsorted = False
            sys.stdout.write(delimiter*colTwoTab + line2[j])
            j=j+1
                
def open_unsort(filename):

    if filename == "-":
        f = sys.stdin
    else:
        f = open(filename, 'r')
    lines = f.readlines()

    f.close()

    n = len(lines)
    if '\n' not in lines[n-1]:
        lines[n-1] = lines[n-1] + '\n'

    return lines

def sort(lines):

    swaped = True
    while swaped:
        swaped = False
        for i in range(len(lines)-1):
            if lines[i] > lines[i+1]:
                temp = lines[i]
                lines[i] = lines[i+1]
                lines[i+1] = temp
                swaped = True

    return lines
    
def main():
    
    version_msg = "%prog 1.0"
    usage_msg = """%prog [OPTION]... FILE FILE 2
Output comparisons between two files."""
        
    parser = OptionParser(version=version_msg, usage=usage_msg)

    parser.add_option("-u", "--unsorted",
                      action="store_true", dest="unsort", default=False,
                      help="sort lines")
    parser.add_option("-1", action="store_true", dest="one", default=False,
                      help="suppress column 1")
    parser.add_option("-2", action="store_true", dest="two", default=False,
                          help="suppress column 2")
    parser.add_option("-3", action="store_true", dest="three", default=False,
                          help="suppress column 3")
    
    options, args = parser.parse_args(sys.argv[1:])
    
    try:
        unsort = bool(options.unsort)
        one = bool(options.one)
        two = bool(options.two)
        three = bool(options.three)
    except:
        parser.error("invalid type: must be boolean".
                     format(options.unsort))
     
    if(len(args) != 2):
        parser.error("wrong number of operands")
    
    file1 = args[0]
    file2 = args[1]
    
    lines1 = []
    lines2 = []
    lines1 = open_unsort(file1)
    lines2 = open_unsort(file2)
	
    if(unsort):
        lines1 = sort(lines1)
        lines2 = sort(lines2)

    output(lines1, lines2, one, two, three)
    
        
if __name__ == "__main__":
    main()
