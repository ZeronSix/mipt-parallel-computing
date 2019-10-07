#!/usr/bin/python3
import argparse
import random

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Field Generator')
    parser.add_argument('-s', '--seed', type=int, default=0,
                        help='RNG seed')
    parser.add_argument('-W', '--width', type=int, default=1000,
                        help='field width')
    parser.add_argument('-H', '--height', type=int, default=1000,
                        help='field height')
    parser.add_argument('-p', '--proba', type=float, default=0.1,
                        help='alive probability')
    parser.add_argument('-o', '--output', type=str, help='output filename',
                        required=True)

    args = parser.parse_args()
    random.seed(args.seed)

    with open(args.output, 'w') as f:
        f.write(f'{args.width} ')
        f.write(f'{args.height} ')

        for _ in range(args.height):
            for _ in range(args.width):
                alive = int(random.random() < args.proba)
                f.write(f'{alive} ')
