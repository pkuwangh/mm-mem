#!/usr/bin/env python3

import argparse
import json
import signal
import subprocess
import sys
import time
from datetime import datetime


def load_job_list(jobfile):
    with open(jobfile, "rt") as fp:
        jobs = json.load(fp)
    # build a event list
    event_timestamp = []
    for start_time, end_time, _ in jobs:
        if start_time not in event_timestamp:
            event_timestamp.append(start_time)
        if end_time not in event_timestamp:
            event_timestamp.append(end_time)
    # scan again to feed info into sorted queue
    event_timestamp.sort()
    events = {x: [] for x in event_timestamp}
    for idx in range(len(jobs)):
        (start_time, end_time, cmd) = jobs[idx]
        if start_time in events:
            events[start_time].append(["launch", idx, cmd])
        if end_time in events:
            events[end_time].append(["terminate", idx, cmd])
    return (len(jobs), event_timestamp, events)


def main(args):
    (num_jobs, event_timestamp, events) = load_job_list(args.job)
    procs = [None for _ in range(num_jobs)]
    try:
        cur_time = 0
        for timestamp in event_timestamp:
            if cur_time < timestamp:
                time.sleep(timestamp - cur_time)
            cur_time = timestamp
            print(datetime.now().strftime("%H:%M:%S"), flush=True)
            for action, idx, cmd in events[timestamp]:
                sys.stdout.flush()
                sys.stderr.flush()
                if action == "launch":
                    p = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, shell=False)
                    assert procs[idx] is None
                    print(f'launch job {idx}: {" ".join(cmd)}', flush=True)
                    procs[idx] = p
                else:
                    assert procs[idx] is not None
                    print(f'kill job {idx}: {" ".join(cmd)}', flush=True)
                    if procs[idx].poll() is None:
                        procs[idx].send_signal(signal.SIGINT)
    except Exception as e:
        print(f"exception: {str(e)}", flush=True)
    finally:
        for p in procs:
            if p and p.poll() is None:
                print(f"final kill {p.pid}", flush=True)
                p.terminate()


def init_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--job", "-j", type=str, required=True, help="json file describing jobs"
    )
    return parser


if __name__ == "__main__":
    parser = init_parser()
    args = parser.parse_args()
    main(args)
