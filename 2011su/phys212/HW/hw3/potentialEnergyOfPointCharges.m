definedVariables
% A point charge $q_2 = -1.8 mu$ C is fixed at the origin of a coordinate system as shown.  Another point charge $q_1 = 0.6 mu$ C is intially located at point $P$, a distance $d_1 = 5.9$ cm from the origin along the x-axis.  

q_2 = -1.8 * mu %C
q_1 = 0.6 * mu %C
d_1 = 5.9/100  %m 

% 1) What is \delta PE, the change in potential energy of charge $q_1$ when it is moved from point $P$ to point $R$, located a distance $d_2 = 2.4$ cm from the origin along the x-axis as shown?

d_2 = 2.4/100 %m 

ans_1 = ((q_1 * q_2)/(4 * pi * epsilon_0))*((1/d_1) - (1/d_2))

% 2) The charge $q_2$ is now replaced by two charges $q_3$ and $q_4$ which each have a magnitude of $-0.9 * mu$ C, half of that of $q_2$.  The charges are located a distance $a = 1.4/100$m from the oringin along the y-axis as shown.  What is \delta PE, the change in potential energy now if charge $q_1$ is moved from point $P$ to point $R$?  

q_3 = -0.9 * mu % C
q_4 = q_3 

a = 1.4/100 %m 

d_diag1 = sqrt( a^2 + d_1^2)
d_diag2 = sqrt( a^2 + d_2^2)

ans_2 = ((q_1 * (q_3 +q_4))/(4 * pi * epsilon_0))*((1/d_diag1) - (1/d_diag2))

% 3) What is the ptential energy of the system composed of thee charges $q_1$, $q_3$ and $q_4$, when $q_1$ is at the point R?  Define the potential enery to be zero at infinity.  


ans_3 = kappa*(((q_3 * q_4)/(2*a)) +((q_3 * q_1)/(d_diag2)) + ((q_4 * q_1)/(d_diag2)))

% 4) The charge $q_4$ is now replaced by charge $q_5$ which has the same magnitude, but opposite sign from $q_4$ (i.e., $q_5 = 0.9 * mu$ C).  What is the new value for the potential energy of the system?  

q_5 = 0.9 * mu %C

ans_4 = kappa*(((q_3 * q_5)/(2*a)) +((q_3 * q_1)/(d_diag2)) + ((q_5 * q_1)/(d_diag2)))

% 5) Charges $q_3$ and $q_5$ are now replaced by two charges, $q_2$ and $q_6$, having equal magnitude and sign ($-1.8 * mu$ C).  Charges $q_2$ is located at the origin and charge $q_6$ is located a distance $d = d_1 + d_2 = 8.3$cm from the origin as shown.  What is \delta PE, the change in potential energy now if charge $q1$ is moved from point $P$ to point $R$?  

q_6 = q_2 % -1.8 * mu %C

d = d_1 + d_2 % 8.3 cm

  
ans_5 = kappa*(((q_2 * q_1)/(d_1)) +((q_6 * q_1)/(d_2)) + ((q_6 * q_2)/(d))) - (kappa*(((q_2 * q_1)/(d_2)) +((q_6 * q_1)/(d_1)) + ((q_6 * q_2)/(d))))
 
