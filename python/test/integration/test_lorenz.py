def test_lorenz():
    from musica.examples.lorenz import main
    Xs, Ys, Zs = main(plot=False)
    n = 2
    expected_points = 2000 - 200  # nsteps - burnout
    assert len(Xs) == n, f"Expected {n} trajectories, got {len(Xs)}"
    assert len(Xs[0]) == expected_points, f"Expected {expected_points} points, got {len(Xs[0])}"
