#!/usr/bin/env python3

import queue
import re

def main():
    que = queue.Queue()
    start_pattern = re.compile(r'^s(.*)$')
    end_pattern = re.compile(r'^e(.*)$')
    while True:
        try:
            line = input()
        except:
            break
        start_match = start_pattern.match(line)
        end_match = end_pattern.match(line)
        if start_match:
            que.put(float(start_match.group(1)))
        elif end_match:
            start = que.get()
            end = float(end_match.group(1))
            print('{}: {}'.format(start, end - start))

if __name__ == '__main__':
    main()
