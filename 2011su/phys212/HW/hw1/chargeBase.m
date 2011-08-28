s = 0.24
mu = 10^(-6)
q = 3 * mu
Q = 4.5 * mu
k = 9 * 10^9

Qminus_x = -k*q*Q/(2*s)^2
Qplus_x = k*q*Q/(2*sqrt(2*s^2))^2*(sqrt(2)/2)
Qplus_y = k*q*Q/(2*sqrt(2*s^2))^2*(sqrt(2)/2)

F_x = Qminus_x + Qplus_x
F_y = Qplus_y
F = sqrt(F_x^2 + F_y^2)
