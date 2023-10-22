import hou

def writeData(filepath, data):
    f = open(filepath, 'w')
    f.write(data)
    f.close()

def main():
    node = hou.selectedNodes()[0]
    geo = node.geometry()
    prims = geo.prims()
    points = geo.points()

    indices_str = ''
    for prim in prims:
        indices_str += '\t' + str(prim.points()[0].number()) + ', ' + str(prim.points()[1].number()) + ', ' + str(prim.points()[2].number()) + ', ' + '\n'
        # print(indices_str)

    vertices_str = ''
    for point in points:
        x = str(point.position()[0]) + 'f'
        y = str(point.position()[1]) + 'f'
        z = str(point.position()[2]) + 'f'
        s = str(point.attribValue('uv')[0]) + 'f'
        t = str(point.attribValue('uv')[1]) + 'f'
        u = str(point.attribValue('uv')[2]) + 'f'
        n = str(point.attribValue('N')[0]) + 'f'
        m = str(point.attribValue('N')[1]) + 'f'
        o = str(point.attribValue('N')[2]) + 'f'
        vertices_str += '\t' + x + ', ' + y + ', ' + z + ', ' + s + ', ' + t + ', ' + u + ', ' + n + ', ' + m + ', ' + o + ',\n'
        # print(vertices_str)

    node = hou.node('.')
    filepath = node.parm('filepath').eval()
    asset_name = filepath.split('/')[-1]
    header = '#include "core.h"\n'
    indices_str = 'u32 ' + asset_name[:-2] + '_indices[] = {\n' + indices_str + '};\n\n'
    vertices_str = 'f32 ' + asset_name[:-2] + '_vertices[] = {\n' + vertices_str + '};\n'
    header += indices_str + vertices_str
    writeData(filepath, header)
