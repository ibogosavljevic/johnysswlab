import sys
import argparse
import subprocess
import math
import locale
import datetime
import os
import shlex
import re


def float2(val):
    """
    Parse a string to a floating point number. Uses locale.atof(),
    in future with ICU present will use icu.NumberFormat.parse().
    """
    return float(val.replace(',', '.'))

def parse_number(str):
    match = re.match(r'^(\D*)\s*([\d.]+)\s*(\D*)$', str)
    if match:
        prefix, number, suffix = match.groups()
        return prefix.strip(), float2(number), suffix.strip()
    else:
        raise ValueError("Input string is not in the expected format '[prefix] number [suffix]'.")

def run_command(command_words, num_runs, save_to_file, silent):
    outputs = []
    pid = os.getpid()
    try:
        os.mkdir("stat")
    except FileExistsError:
        print("")

    for i in range(num_runs):
        print("+++ Running %d/%d +++" % ((i + 1), num_runs))
        output_str = []
        with subprocess.Popen(shlex.split(command_words[0]), stdout=subprocess.PIPE, universal_newlines=True) as p:
            for line in p.stdout:
                if not silent:
                    print(line, end = '')
                output_str.append(line)

        outputs.append(output_str)

        if save_to_file:
            text_file = open("stat/stat-{0}-{1}.txt".format(pid, i), "w")
            for l in output_str:
                text_file.write(l)
                text_file.write("\n")
            text_file.close()

    return outputs

def verify_line_counts(outputs, num_runs):
    line_cnt = len(outputs[0])
    for i in range(num_runs):
        if line_cnt != len(outputs[i]):
            print("Different line counts: {}, {}".format(line_cnt, len(outputs[i])))
            print(outputs[0])
            print
            return False
    
    return True


def tokenize_line(line):
    start = 0
    current = 0
    tokens = []
    is_blank = True

    while current < len(line):
        if current == start:
            is_blank = line[current].isspace()    
        
        if line[current].isspace() == is_blank:
            current += 1
        else:
            tokens.append(line[start:current])
            start = current
    
    tokens.append(line[start:current])

    # Remove trailing whitespace 
    if len(tokens) > 1:
        if tokens[len(tokens) - 1].isspace():
            del tokens[-1]

    return tokens


def main():
    parser = argparse.ArgumentParser(description='Description of your program')
    parser.add_argument('-n','--num-runs', help='Number of time to run your program', required=True, type=int)
    parser.add_argument('-c','--command', help='Command to execute', nargs='+',required=True)
    parser.add_argument('-f','--save-to-file', help='Save output to file', required=False, default=True, type=bool)
    parser.add_argument('-s','--silent', help='Don\'t display info on the screen', required=False, default=False, type=bool)
    parser.add_argument('-p','--precision', help='Precision (in number of digit)', required=False, default=3, type=int)
    args = vars(parser.parse_args())

    num_runs = args['num_runs']
    command_words = args['command']
    save_to_file = args['save_to_file']
    silent = args['silent']
    precision = args['precision']
    
    # outputs[n][i] refers to the i-th line in n-th execution of the command  
    outputs = run_command(command_words, num_runs, save_to_file, silent)

    if not verify_line_counts(outputs, num_runs):
        print("Line count missmatch")
        return -1

    # Check that each output has the same number of lines
    line_cnt = len(outputs[0])

    # Go line by line, collect all the numbers
    output_text = ""
    for i in range(line_cnt):
        line_per_exec = []

        # Split line according to 
        for n in range(num_runs):
            line_per_exec.append(tokenize_line(outputs[n][i]))

        word_cnt = len(line_per_exec[0])
        skip_line = False

        for n in range(num_runs):
            if word_cnt != len(line_per_exec[n]):
                print("Word count missmatch on line %d" % (i + 1))
                print("Original word count %d, actual %d" % (word_cnt, len(line_per_exec[n])))
                print(line_per_exec[0])
                print(line_per_exec[n])
                skip_line = True

        if skip_line:
            output_text += "XXXXXXXXXXXXX\n"
            continue
      
        for i in range(word_cnt):
            # For each word, check if same. If same, do not modify
            word = line_per_exec[0][i]
            same = True
            blank_word = word.isspace()

            for n in range(num_runs):
                if line_per_exec[n][i] != word:
                    same = False
                    break

            if same:
                output_text += word
            elif blank_word:
                output_text += word
            else:
                values = []
                parsed = True
                prefix = ""
                suffix = ""
                try:
                    prefix, tmp, suffix = parse_number(line_per_exec[0][i])
                    for n in range(num_runs):
                        prefix0, val0, suffix0 = parse_number(line_per_exec[n][i])
                        if ((prefix == prefix0) and (suffix == suffix0)):
                            values.append(val0)
                        else:
                            raise ValueError("Mismatched prefixes or suffixes")
                except ValueError:
                    output_text += "XX" + line_per_exec[0][i] + "XX"
                    parsed = False

                if parsed:
                    values.sort()
                    # Calculate mean
                    sum = 0.0
                    for i in range(len(values)):
                        sum += values[i]

                    mean = sum / len(values)

                    # Calculate standard deviation
                    sum = 0.0
                    for i in range(len(values)):
                        diff = (values[i] - mean)
                        sum += diff * diff

                    stddev = math.sqrt(sum / len(values))

                    max = values[len(values) - 1]
                    min = values[0]
                    median = 0.0
                    if len(values) %2 != 0:
                        median = values[len(values)//2]
                    else:
                        right = len(values) // 2
                        left = right - 1
                        median = (values[left] + values[right]) / 2

                    output_text += prefix
                    output_text += ' {0:.{1}f}'.format(mean, precision) + " (" + '{0:.{1}f}'.format(stddev, precision) + ")" 
                    output_text += '[ {0:.{1}f}'.format(min, precision) + ', {0:.{1}f}'.format(median, precision) + ', {0:.{1}f} ] '.format(max, 3) 
                    output_text += suffix
            
       
        output_text += ("\n")
            

    print(output_text)

if __name__ == "__main__":
    sys.exit(main())
