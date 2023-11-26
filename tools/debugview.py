from pymxs import runtime as rt # pylint: disable=import-error

MAX_SYSTEM_SCENE_UNIT = 100.0
VOXEL_UNIT = 2.0

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

    rt.setVisibility(voxel, 0.5)

    voxel.length = VOXEL_UNIT * MAX_SYSTEM_SCENE_UNIT
    voxel.width = VOXEL_UNIT * MAX_SYSTEM_SCENE_UNIT
    voxel.height = VOXEL_UNIT * MAX_SYSTEM_SCENE_UNIT

    voxel.name = 'v' + str(id)
    voxel.pos = rt.Point3(x * MAX_SYSTEM_SCENE_UNIT, y * MAX_SYSTEM_SCENE_UNIT, z * MAX_SYSTEM_SCENE_UNIT)
    if inside:
        red = rt.globalVars.get(rt.name('red'))
        voxel.wirecolor = red
    else:
        green = rt.globalVars.get(rt.name('green'))
        voxel.wirecolor = green
        voxel.isNodeHidden = True
    return voxel
    

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

def main():
    init_globals()
    # print_controllers()

    voxel_file = 'D:\\Graphics\\Models\\GLTF2\\voxels.txt'
    voxelize(voxel_file)

main()
