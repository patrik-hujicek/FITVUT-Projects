import sys


def fatal_error(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)
    exit(1)
