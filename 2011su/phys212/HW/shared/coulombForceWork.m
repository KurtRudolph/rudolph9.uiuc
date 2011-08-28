function W_AB = coulombForceWork(q_1, q_2, epsilon_0, r_A, r_B)

W_AB = ((q_1 * q_2)/(4*pi*epsilon_0))*((1/r_A) - (1/r_B))

% Note:  no matter what the path of particle B the work is always the same
