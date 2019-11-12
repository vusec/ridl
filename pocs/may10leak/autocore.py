#!/usr/bin/env python3

def get_hypergroups():
    groups = {}
    cpu_id = None
    core_id = None

    with open('/proc/cpuinfo') as f:
        for line in f.readlines():
            if line.startswith('processor'):
                cpu_id = int(line.split(':', 1)[-1])

            if line.startswith('core id'):
                core_id = int(line.split(':', 1)[-1])

            if cpu_id is None or core_id is None:
                continue

            groups[core_id] = groups.get(core_id, []) + [cpu_id]
            cpu_id = None
            core_id = None

    return groups.values()

def main():
    for group in get_hypergroups():
        print(group)

if __name__ == '__main__':
    main()
