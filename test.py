import area

a = area.new()
area.add_point(a, 0, 10, 10, 0, 0)
area.add_point(a, 0, 30, 10, 0, 0)
area.add_point(a, 0, 30, 30, 0, 0)
area.add_point(a, 0, 10, 30, 0, 0)
area.add_point(a, 0, 10, 10, 0, 0)

area.offset(a, 3)
for curve in range(0, area.num_curves(a)):
    for vertex in range(0, area.num_vertices(a, curve)):
        sp, x, y, cx, cy = area.get_vertex(a, curve, vertex)
        print sp, x, y, cx, cy

