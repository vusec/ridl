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

def get_victim():
    l = list(get_hypergroups())
    if len(l) > 1:
        return(l[1][0])
    return l[0][0]

def get_attacker():
    l = list(get_hypergroups())
    if len(l) > 1:
        return(l[1][1])
    return l[0][1]



if __name__ == "__main__":
    print(get_victim())
