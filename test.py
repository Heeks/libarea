import area

a1 = area.new()
area.add_point(a1, 0, 10, 10, 0, 0)
area.add_point(a1, 0, 30, 10, 0, 0)
area.add_point(a1, 0, 30, 30, 0, 0)
area.add_point(a1, 0, 10, 30, 0, 0)
area.add_point(a1, 0, 10, 10, 0, 0)

area.offset(a1, 3)
for curve in range(0, area.num_curves(a1)):
    for vertex in range(0, area.num_vertices(a1, curve)):
        sp, x, y, cx, cy = area.get_vertex(a1, curve, vertex)
        print sp, x, y, cx, cy

