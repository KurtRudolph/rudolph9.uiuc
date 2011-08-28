function F_E = coulombForce( epsilon_0, Q, q, r, r_hat)

F_E = (1/(4*pi*epsilon_0))*((Q*q)/r^2)*r_hat
