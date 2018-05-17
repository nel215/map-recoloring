import os
import logging
import subprocess
import numpy as np
from tqdm import tqdm
from datetime import datetime
from concurrent.futures import ProcessPoolExecutor
logger = logging.getLogger(__name__)


def run_experiment(args):
    seed, logdir = args
    logpath = os.path.join(logdir, '%04d.ltsv' % (seed))
    args = [
        'java',
        'MapRecoloringVis',
        '-novis',
        '-exec',
        './a.out',
        '-seed',
        str(seed),
    ]
    with open(logpath, 'w') as fp:
        subprocess.run(args, stdout=fp)


def parse_log(args):
    seed, logdir = args
    logpath = os.path.join(logdir, '%04d.ltsv' % (seed))

    bestUsed = 1e9
    bestRecolor = 40000
    with open(logpath) as fp:
        for row in fp:
            d = {}
            if row.find('Score =') >= 0:
                continue
            if row.find(':') < 0:
                continue
            for chunk in row.strip().split('\t'):
                k, v = chunk.split(':')
                d[k] = v
            if 'H' in d:
                H = float(d['H'])
            if 'W' in d:
                W = float(d['W'])
            if 'bestUsed' in d:
                bestUsed = float(d['bestUsed'])
            if 'bestRecolor' in d:
                bestRecolor = float(d['bestRecolor'])

    return bestUsed * (bestRecolor/H/W)


def run_experiments():
    now = datetime.now()
    logdir = os.path.join('./log', now.strftime('%Y%m%d-%H%M%S'))
    os.makedirs(logdir, exist_ok=True)
    executor = ProcessPoolExecutor()
    args_list = []
    for i in range(100):
        args_list.append([i+1, logdir])
    total = len(args_list)
    for res in tqdm(
        executor.map(run_experiment, args_list), total=total, ncols=0
    ):
        pass

    scores = []
    for res in tqdm(executor.map(parse_log, args_list), total=total, ncols=0):
        scores.append(res)

    scores = np.array(scores)
    mean = np.mean(scores)
    std = np.std(scores)
    logger.info('mean: {}, std: {}'.format(mean, std))


def main():
    logging.basicConfig(level=logging.INFO)
    run_experiments()


if __name__ == '__main__':
    main()
