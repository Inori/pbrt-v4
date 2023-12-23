from pymxs import runtime as rt # pylint: disable=import-error

MAX_SYSTEM_SCENE_UNIT = 100.0
VOXEL_UNIT = 1.0

def init_globals():

    # Idealy, it shoule be like the following, but I don't why this crashes.
    # vis = rt.getSubAnim(voxel, 1)
    # rt.setProperty(vis, 'controller', rt.noise_float())
    rt.execute('''fn setVisibility node vis = (
               node.visibility = bezier_float()
               node.visibility.controller.value = vis
               )
               ''')

def debug_print(msg):
    with open('max_debug.log', 'w+') as dst:
        dst.write(msg + '\n')

def print_controllers():
    b = rt.Box()
    
    lines = []
    for sub_i in range(b.numsubs):
        sub = rt.getSubAnim(b, sub_i+1)
        lines.append(f'{sub.name} [{sub_i+1}]\n')

        if sub.controller == None:
            lines.append(' No controller assigned to it\n')
        else:
            lines.append(f' current controller: {(sub.controller)}\n')
        if sub.keys == None:
            lines.append(' No keys assigned to it\n')

        for secsub_i in range(sub.numsubs):
            secsub = rt.getSubAnim(sub, secsub_i+1)
            lines.append(f'\t{secsub.name} [{secsub_i+1}]\n')

            if secsub.controller == None:
                lines.append('\t No controller assigned to it\n')
            else:
                lines.append(f'\t current controller: {(secsub.controller)}\n')
            if secsub.keys == None:
                lines.append('\t No keys assigned to it\n')
            for thirdsub_i in range(secsub.numsubs):
                thirdsub = rt.getSubAnim(secsub, thirdsub_i+1)
                lines.append(f'\t\t{thirdsub.name} [{thirdsub_i+1}]\n')
                if secsub.controller == None:
                    lines.append('\t No controller assigned to it\n')
                else:
                    lines.append(f'\t\t current controller: {(thirdsub.controller)}\n')
    
    with open('box_controller.txt', 'w') as dst:
        dst.writelines(lines)
   

def create_voxel(id, x, y, z, inside):
    if not inside:
        return None
    
    voxel = rt.Box()

    voxel.name = 'v' + str(id)
    rt.setVisibility(voxel, 0.8)

    # scale object first
    voxel.length = VOXEL_UNIT * MAX_SYSTEM_SCENE_UNIT
    voxel.width = VOXEL_UNIT * MAX_SYSTEM_SCENE_UNIT
    voxel.height = VOXEL_UNIT * MAX_SYSTEM_SCENE_UNIT

    # then move pivot to cube center, default is on the bottom
    voxel.pivot = voxel.center

    # finaly, move object to position
    voxel.pos = rt.Point3(x * MAX_SYSTEM_SCENE_UNIT, y * MAX_SYSTEM_SCENE_UNIT, z * MAX_SYSTEM_SCENE_UNIT)
    if inside:
        red = rt.globalVars.get(rt.name('red'))
        voxel.wirecolor = red
    else:
        green = rt.globalVars.get(rt.name('green'))
        voxel.wirecolor = green
        voxel.isNodeHidden = True
    return voxel

def create_probe(id, x, y, z):
    probe = rt.Sphere()

    probe.name = 'p' + str(id)
    rt.setVisibility(probe, 0.9)

    # scale object first
    probe.radius = 0.1 * MAX_SYSTEM_SCENE_UNIT

    # finaly, move object to position
    probe.pos = rt.Point3(x * MAX_SYSTEM_SCENE_UNIT, y * MAX_SYSTEM_SCENE_UNIT, z * MAX_SYSTEM_SCENE_UNIT)
    green = rt.globalVars.get(rt.name('green'))
    probe.wirecolor = green
    return probe

def create_sphere(name, color, radius, x, y, z):
    sphere = rt.Sphere()

    sphere.name = name
    #rt.setVisibility(sphere, 0.9)

    # scale object first
    sphere.radius = radius * MAX_SYSTEM_SCENE_UNIT

    # finaly, move object to position
    sphere.pos = rt.Point3(x * MAX_SYSTEM_SCENE_UNIT, y * MAX_SYSTEM_SCENE_UNIT, z * MAX_SYSTEM_SCENE_UNIT)
    c = rt.globalVars.get(rt.name(color))
    sphere.wirecolor = c
    return sphere
    

def voxelize(src_file):
    dummy_group = rt.Dummy()
    dummy_group.name = 'voxels'
    with open(src_file) as src:
        for line in src.readlines():
            line = line.rstrip('\n')
            if not line:
                continue
            parts = line.split(' ')
            if len(parts) != 5:
                print('error voxel format: {}'.format(line))
                continue
            id = int(parts[0])
            x = float(parts[1])
            y = float(parts[2])
            z = float(parts[3])
            inside = int(parts[4])
            voxel = create_voxel(id, x, y, z, inside)
            if voxel:
                voxel.Parent = dummy_group


def place_probes(src_file):
    dummy_group = rt.Dummy()
    dummy_group.name = 'probes'
    with open(src_file) as src:
        for line in src.readlines():
            line = line.rstrip('\n')
            if not line:
                continue
            parts = line.split(' ')
            if len(parts) != 4:
                print('error probe format: {}'.format(line))
                continue
            id = int(parts[0])
            x = float(parts[1])
            y = float(parts[2])
            z = float(parts[3])
            probe = create_probe(id, x, y, z)
            if probe:
                probe.Parent = dummy_group

def show_dirs(dir_file):
    create_sphere('unit', 'gray' ,1 , 0, 0, 0)
    with open(dir_file) as src:
        index = 0
        for line in src.readlines():
            line = line.rstrip('\n')
            if not line:
                continue
            parts = line.split(' ')
            if len(parts) != 3:
                print('error dir format: {}'.format(line))
                continue
            x = float(parts[0])
            y = float(parts[1])
            z = float(parts[2])
            name = 's_{}'.format(index)
            index += 1
            create_sphere(name, 'green', 0.01, x, y, z)

def main():
    init_globals()
    # print_controllers()

    # voxel_file = 'D:\\Graphics\\Models\\GLTF2\\voxels.txt'
    # voxelize(voxel_file)

    # probe_file = 'D:\\Graphics\\Models\\GLTF2\\probes.txt'
    # place_probes(probe_file)

    dir_file = 'D:\\Graphics\\Models\\GLTF2\\sample_dirs.txt'
    show_dirs(dir_file)

main()
