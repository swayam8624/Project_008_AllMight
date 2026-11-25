#!/usr/bin/env python3
"""Independent numerical witnesses for Book03 P35; requires NumPy.

These check selected derivations, not the complete reader or native library.
"""
import numpy as np


def ray_triangle(a, b, c, o, d):
    e1, e2, t = b - a, c - a, o - a
    p, q = np.cross(d, e2), np.cross(t, e1)
    det = e1 @ p
    if abs(det) <= 1e-12:  # bounded, deliberately well-scaled fixtures only
        return None
    return np.array([t @ p, d @ q, e2 @ q]) / det


def rotation_fit(x, y):
    xc, yc = x.mean(axis=0), y.mean(axis=0)
    u, _, vt = np.linalg.svd((x-xc).T @ (y-yc))
    v = vt.T
    correction = np.diag([1., 1., np.linalg.det(v @ u.T)])
    r = v @ correction @ u.T
    return r, yc-r@xc


def address(i, size, mode):
    if mode == 'repeat':
        return i % size
    if mode == 'clamp':
        return min(max(i, 0), size-1)
    q = i % (2*size)
    return q if q < size else 2*size-1-q


def sample(u, texels, mode):
    x = u*len(texels)-.5
    i = int(np.floor(x))
    a = x-i
    return (1-a)*texels[address(i, len(texels), mode)] + a*texels[address(i+1, len(texels), mode)]


def main():
    a, b, c = np.array([[0.,0,0],[2,0,0],[0,2,0]])
    d = np.array([0.,0,-1])
    assert np.allclose(ray_triangle(a,b,c,np.array([.5,.5,3]),d), [.25,.25,3])
    assert np.allclose(ray_triangle(a,b,c,np.array([1.5,1.5,3]),d), [.75,.75,3])
    assert ray_triangle(a,b,c,np.array([.5,.5,3]),np.array([1.,0,0])) is None
    rng = np.random.default_rng(35)
    errors = []
    for _ in range(500):
        # Reject ill-conditioned examples; this is not a watertightness test.
        while True:
            a,b,c,o,d = rng.uniform(-2,2,(5,3))
            matrix = np.column_stack((b-a,c-a,-d))
            if np.linalg.cond(matrix) < 100:
                break
        got = ray_triangle(a,b,c,o,d)
        expected = np.linalg.solve(matrix,o-a)
        errors.append(np.max(np.abs(got-expected)))
    assert max(errors)<1e-10

    # Square with a zero-weight topological diagonal.
    p = np.array([[0.,0,0],[1,0,0],[1,1,0],[0,1,0]])
    faces = [(0,1,2),(0,2,3)]
    k = np.zeros((4,4)); mass = np.zeros(4)
    for f in faces:
        area = np.linalg.norm(np.cross(p[f[1]]-p[f[0]], p[f[2]]-p[f[0]]))/2
        for i in f:
            mass[i] += area/3
        for i,j,l in [(f[0],f[1],f[2]),(f[1],f[2],f[0]),(f[2],f[0],f[1])]:
            x,y = p[i]-p[l],p[j]-p[l]
            w = (x@y)/(2*np.linalg.norm(np.cross(x,y)))
            k[i,i]+=w; k[j,j]+=w; k[i,j]-=w; k[j,i]-=w
    assert k[0,2] == 0 and np.allclose(k[[0,1,2,3],[1,2,3,0]],-.5)
    assert np.allclose(k@np.ones(4),0)
    s = k/np.sqrt(mass[:,None]*mass[None,:])
    eigenvalues = np.linalg.eigvalsh(s)
    assert eigenvalues.min()>-1e-12
    step = 2/eigenvalues.max()
    assert np.max(np.abs(1-step*eigenvalues)) <= 1+1e-12
    assert np.max(np.abs(1-1.01*step*eigenvalues)) > 1

    modes = ['repeat','clamp','mirror']
    seam = [sample(0,[0.,1.],mode) for mode in modes]
    assert seam == [.5,0.,0.]
    assert [sample(.5,[0.,1.],mode) for mode in modes] == [.5,.5,.5]
    for size in range(1,10):
        for i in range(-40,41):
            for mode in modes:
                assert 0 <= address(i,size,mode) < size

    x = np.array([[0.,0,0],[1,0,0],[0,1,0]])
    y = np.array([[2.,3,4],[2,4,4],[1,3,4]])
    true_r = np.array([[0.,-1,0],[1,0,0],[0,0,1]])
    r,t = rotation_fit(x,y)
    assert np.allclose(r,true_r) and np.allclose(t,[2,3,4])
    assert np.allclose(x@r.T+t,y)
    # A noncoplanar asymmetric cloud reflected through a plane cannot be
    # reproduced exactly by a proper rotation and translation.
    x = np.array([[0.,0,0],[1,0,0],[0,2,0],[0,0,3]])
    y = x @ np.diag([-1.,1,1])
    r,t = rotation_fit(x,y)
    assert np.allclose(r.T@r,np.eye(3)) and abs(np.linalg.det(r)-1)<1e-12
    reflection_rmse = np.sqrt(np.mean(np.sum((x@r.T+t-y)**2,axis=1)))
    assert reflection_rmse > .1
    print('P35 PASS ray_solver_max_abs_error=',max(errors))
    print('cotangent_mass_scaled_eigenvalues=',eigenvalues.tolist(),'stable_step_limit=',step)
    print('texture_seam_repeat_clamp_mirror=',seam)
    print('proper_rotation_reflection_rmse=',reflection_rmse)


if __name__ == '__main__':
    main()
